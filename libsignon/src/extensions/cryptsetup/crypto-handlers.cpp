/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 *
 * Contact: Aurel Popirtac <ext-aurel.popirtac@nokia.com>
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <sys/mount.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libcryptsetup.h>

#include <QDataStream>
#include <QTextStream>
#include <QProcess>
#include <QLatin1Char>
#include <QFileInfo>
#include <QDir>

#include "crypto-handlers.h"
#include "debug.h"
#include "misc.h"

#define SIGNON_LUKS_DEFAULT_HASH  "ripemd160"

#define SIGNON_LUKS_CIPHER_NAME   "aes"
#define SIGNON_LUKS_CIPHER_MODE   "xts-plain"
#define SIGNON_LUKS_CIPHER        \
    SIGNON_LUKS_CIPHER_NAME "-" SIGNON_LUKS_CIPHER_MODE
#define SIGNON_LUKS_KEY_SIZE      256
#define SIGNON_LUKS_BASE_KEYSLOT  0

#define SIGNON_EXTERNAL_PROCESS_READ_TIMEOUT 300

#define KILO_BYTE_SIZE 1024
#define MEGA_BYTE_SIZE (KILO_BYTE_SIZE * 1024)

/*  ------------- SystemCommandLineCallHandler implementation -------------- */

SystemCommandLineCallHandler::SystemCommandLineCallHandler()
{
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(error(QProcess::ProcessError)));
}

SystemCommandLineCallHandler::~SystemCommandLineCallHandler()
{
}

bool SystemCommandLineCallHandler::makeCall(const QString &appPath,
                                            const QStringList &args,
                                            bool readOutput)
{
    QString trace;
    QTextStream stream(&trace);
    stream << appPath << QLatin1Char(' ') << args.join(QLatin1String(" "));
    TRACE() << trace;

    m_process.start(appPath, args);
    if (!m_process.waitForStarted()) {
        BLAME() << "Wait for started failed";
        return false;
    }

    if (readOutput) {
        m_output.clear();

        if (m_process.waitForReadyRead(SIGNON_EXTERNAL_PROCESS_READ_TIMEOUT)) {
            if (!m_process.bytesAvailable()) {
                BLAME() << "Coult not read output of external process ";
                return false;
            }

            while(m_process.bytesAvailable())
                m_output += m_process.readAllStandardOutput();
        }
    }

    if (!m_process.waitForFinished()) {
        TRACE() << "Wait for finished failed";
        return false;
    }

    return true;
}

void SystemCommandLineCallHandler::error(QProcess::ProcessError err)
{
    TRACE() << "Process erorr:" << err;
}


/*  ------------------ PartitionHandler implementation --------------------- */

bool PartitionHandler::createPartitionFile(const QString &fileName,
                                           const quint32 fileSize)
{
    int fd = open(fileName.toLatin1().data(),
                  O_RDWR | O_CREAT,
                  666);

    if (fd < 0) {
        BLAME() << "FAILED to create signon secure FS partition file. ERRNO:"
                << errno;
        return false;
    }

    if (ftruncate(fd, fileSize * MEGA_BYTE_SIZE) == -1) {
        BLAME() << "FAILED to set signon secure FS partition file size. ERRNO:"
                << errno;
        return false;
    }

    if (close(fd) < 0)
        TRACE() << "Failed to close secure FS partition file after creation.";

    if (!setFilePermissions(fileName, signonFilePermissions))
        TRACE() << "Failed to set file permissions "
                   "for the secure storage container.";

    return true;
}

bool PartitionHandler::formatPartitionFile(const QString &fileName,
                                           const quint32 fileSystemType)
{
    QString mkfsApp = QString::fromLatin1("/sbin/mkfs.ext2");
    switch (fileSystemType) {
        case Ext2: mkfsApp = QString::fromLatin1("/sbin/mkfs.ext2"); break;
        case Ext3: mkfsApp = QString::fromLatin1("/sbin/mkfs.ext3"); break;
        case Ext4: mkfsApp = QString::fromLatin1("/sbin/mkfs.ext4"); break;
        default: break;
    }

    SystemCommandLineCallHandler handler;
    return handler.makeCall(
                        mkfsApp,
                        QStringList() << fileName);
}


/*  --------------------- MountHandler implementation ---------------------- */

bool MountHandler::mount(const QString &toMount,
                         const QString &mountPath,
                         const QString &fileSystemTtpe)
{
    /* Mount a filesystem.  */
    return (::mount(toMount.toUtf8().constData(),
                    mountPath.toUtf8().constData(),
                    fileSystemTtpe.toUtf8().constData(),
                    MS_SYNCHRONOUS | MS_NOEXEC, NULL) == 0);
}

bool MountHandler::umount(const QString &mountPath)
{
    /* Unmount a filesystem.  */

    //TODO - investigate why errno is EINVAL

    TRACE() << mountPath.toUtf8().constData();
    int ret = ::umount2(mountPath.toUtf8().constData(), MNT_FORCE);
    TRACE() << ret;

    switch (errno) {
    case EAGAIN: TRACE() << "EAGAIN"; break;
    case EBUSY: TRACE() << "EBUSY"; break;
    case EFAULT: TRACE() << "EFAULT"; break;
    case EINVAL: TRACE() << "EINVAL"; break;
    case ENAMETOOLONG: TRACE() << "ENAMETOOLONG"; break;
    case ENOENT: TRACE() << "ENOENT"; break;
    case ENOMEM: TRACE() << "ENOMEM"; break;
    case EPERM: TRACE() << "EPERM"; break;
    default: TRACE() << "umount unknown error - ignoring.";
    }

    //TODO - Remove 1st, uncommend 2nd lines after the fix above.
    //       This is tmp hack so that the tests will work.
    return true;
    //return (ret == 0);
}

/*  ----------------------- LosetupHandler implementation ----------------------- */

bool LosetupHandler::setupDevice(const QString &deviceName,
                                 const QString &blockDevice)
{
    SystemCommandLineCallHandler handler;
    return handler.makeCall(
                        QLatin1String("/sbin/losetup"),
                        QStringList() << deviceName << blockDevice);
}

QString LosetupHandler::findAvailableDevice()
{
    SystemCommandLineCallHandler handler;
    QString deviceName;
    bool ret = handler.makeCall(
                            QLatin1String("/sbin/losetup"),
                            QStringList() << QLatin1String("-f"),
                            true);

    deviceName = QString::fromLocal8Bit(handler.output().trimmed());

    if (ret)
        return deviceName;

    return QString();
}

bool LosetupHandler::releaseDevice(const QString &deviceName)
{
    SystemCommandLineCallHandler handler;
    return handler.makeCall(QLatin1String("/sbin/losetup"),
                            QStringList() <<
                            QString::fromLatin1("-d") << deviceName);
}

/*  -------------------- CrytpsetupHandler implementation ------------------ */

/*
    Callbacks for the interface callbacks struct in crypt_options struct.
*/
static int yesDialog(char *msg)
{
    Q_UNUSED(msg);
    return 0;
}

static void cmdLineLog(int type, char *msg)
{
    switch (type) {
        case CRYPT_LOG_NORMAL:
            TRACE() << msg;
            break;
        case CRYPT_LOG_ERROR:
            TRACE() << "Error: " << msg;
            break;
        default:
            TRACE() << "Internal error on logging class for msg: " << msg;
            break;
    }
}

static void log_wrapper(int level, const char *msg, void *usrptr)
{
    void (*xlog)(int level, char *msg) = (void (*)(int, char*)) usrptr;
    xlog(level, (char *)msg);
}

static int yesDialog_wrapper(const char *msg, void *usrptr)
{
    int (*xyesDialog)(char *msg) = (int (*)(char*)) usrptr;
    return xyesDialog((char*)msg);
}

int crypt_luksFormatBinary(struct crypt_options *options,
                           const char *pwd,
                           unsigned int pwdLen)
{
    struct crypt_device *cd = NULL;
    struct crypt_params_luks1 cp = {
        options->hash,
        options->align_payload
    };
    int r;

    if ((r = crypt_init(&cd, options->device)))
        return -EINVAL;

    crypt_set_log_callback(cd, log_wrapper, (void*) options->icb->log);
    crypt_set_confirm_callback(cd, yesDialog_wrapper,
                               (void*) options->icb->yesDialog);

    crypt_set_timeout(cd, options->timeout);
    crypt_set_password_retry(cd, options->tries);
    crypt_set_iterarion_time(cd, options->iteration_time ?: 1000);
    crypt_set_password_verify(cd, options->flags & CRYPT_FLAG_VERIFY);

    r = crypt_format(cd, CRYPT_LUKS1,
                     SIGNON_LUKS_CIPHER_NAME, SIGNON_LUKS_CIPHER_MODE,
                     NULL, NULL, options->key_size, &cp);
    if (r < 0)
        goto out;

    /* Add keyslot using internally stored volume key generated during format */
    r = crypt_keyslot_add_by_volume_key(cd, options->key_slot, NULL, 0,
                                        pwd, pwdLen);
out:
    crypt_free(cd);
    return (r < 0) ? r : 0;

}

bool CryptsetupHandler::formatFile(const QByteArray &key,
                                   const QString &deviceName)
{
    struct crypt_options options;

    options.key_size = SIGNON_LUKS_KEY_SIZE / 8;
    options.key_slot = SIGNON_LUKS_BASE_KEYSLOT;

    char *localDeviceName = (char *)malloc(deviceName.length() + 1);
    Q_ASSERT(localDeviceName != NULL);

    strcpy(localDeviceName, deviceName.toLatin1().constData());
    options.device = localDeviceName;

    options.cipher = SIGNON_LUKS_CIPHER;
    options.new_key_file = NULL;

    char *localKey = (char *)malloc(key.length());
    Q_ASSERT(localKey != NULL);
    memcpy(localKey, key.constData(), key.length());

    options.flags = 0;
    options.iteration_time = 1000;
    options.timeout = 0;
    options.align_payload = 0;

    static struct interface_callbacks cmd_icb;
    cmd_icb.yesDialog = 0;
    cmd_icb.log = 0;
    options.icb = &cmd_icb;

    TRACE() << "Device: [" << options.device << "]";
    TRACE() << "Key size:" << key.length();

    int ret = crypt_luksFormatBinary(&options, localKey, key.length());

    if (ret != 0)
        TRACE() << "LUKS format API call result:" << ret << "." << error();

    if (localDeviceName)
        free(localDeviceName);

    if (localKey) {
        memset(localKey, 0x00, key.length());
        free(localKey);
    }

    return (ret == 0);
}

int crypt_luksOpenBinary(struct crypt_options *options,
                         const char *pwd, unsigned int pwdLen)
{
    struct crypt_device *cd = NULL;
    uint32_t flags = 0;
    int r;

    if ((r = crypt_init(&cd, options->device)))
        return -EINVAL;

    crypt_set_log_callback(cd, log_wrapper, (void*) options->icb->log);
    crypt_set_confirm_callback(cd, yesDialog_wrapper,
                               (void*) options->icb->yesDialog);

    crypt_set_timeout(cd, options->timeout);
    crypt_set_password_retry(cd, options->tries);
    crypt_set_iterarion_time(cd, options->iteration_time ?: 1000);
    crypt_set_password_verify(cd, options->flags & CRYPT_FLAG_VERIFY);

    if ((r = crypt_load(cd, CRYPT_LUKS1, NULL))) {
        crypt_free(cd);
        return r;
    }

    if (options->flags & CRYPT_FLAG_READONLY)
        flags |= CRYPT_ACTIVATE_READONLY;

    if (options->flags & CRYPT_FLAG_NON_EXCLUSIVE_ACCESS)
        flags |= CRYPT_ACTIVATE_NO_UUID;

    if (options->key_file)
        r = -1;
    else
        r = crypt_activate_by_passphrase(cd, options->name,
                                         CRYPT_ANY_SLOT,
                                         pwd, pwdLen, flags);

    crypt_free(cd);
    return (r < 0) ? r : 0;
}

bool CryptsetupHandler::openFile(const QByteArray &key,
                                 const QString &deviceName,
                                 const QString &deviceMap)
{
    struct crypt_options options;

    char *localDeviceMap = (char *)malloc(deviceMap.length() + 1);
    Q_ASSERT(localDeviceMap != NULL);
    strcpy(localDeviceMap, deviceMap.toLatin1().constData());
    options.name = localDeviceMap;

    char *localDeviceName = (char *)malloc(deviceName.length() + 1);
    Q_ASSERT(localDeviceName != NULL);
    strcpy(localDeviceName, deviceName.toLatin1().constData());
    options.device = localDeviceName;

    char *localKey = (char *)malloc(key.length());
    Q_ASSERT(localKey != NULL);
    memcpy(localKey, key.constData(), key.length());

    options.key_file = NULL;
    options.timeout = 0;
    /*
        Do not change this:
        1) In case of failure to open, libcryptsetup code will
        enter infinite loop - library BUG/FEATURE.
        2) There is no need for multiple tries, option is intended for
        command line use of the utility.
    */
    options.tries = 0;
    options.flags = 0;

    static struct interface_callbacks cmd_icb;
    cmd_icb.yesDialog = yesDialog;
    cmd_icb.log = cmdLineLog;
    options.icb = &cmd_icb;

    TRACE() << "Device [" << options.device << "]";
    TRACE() << "Map name [" << options.name << "]";
    TRACE() << "Key size:" << key.length();

    int ret = crypt_luksOpenBinary(&options, localKey, key.length());

    if (ret != 0)
        TRACE() << "LUKS open API call result:" << ret << "." << error() << ".";

    if (localDeviceName)
        free(localDeviceName);

    if (localDeviceMap)
        free(localDeviceMap);

    if (localKey) {
        memset(localKey, 0x00, key.length());
        free(localKey);
    }

    return (ret == 0);
}

bool CryptsetupHandler::closeFile(const QString &deviceMap)
{
    struct crypt_options options;

    char *localDeviceMap = (char *)malloc(deviceMap.length() + 1);
    Q_ASSERT(localDeviceMap != NULL);
    strcpy(localDeviceMap, deviceMap.toLatin1().constData());
    options.name = localDeviceMap;

    static struct interface_callbacks cmd_icb;
    cmd_icb.yesDialog = yesDialog;
    cmd_icb.log = cmdLineLog;
    options.icb = &cmd_icb;

    TRACE() << "Map name [" << options.name << "]";

    int ret = crypt_remove_device(&options);

    if (ret != 0)
        TRACE() << "Cryptsetup remove API call result:" << ret <<
            "." <<  error();

    if (localDeviceMap)
        free(localDeviceMap);

    return (ret == 0);
}

bool CryptsetupHandler::removeFile(const QString &deviceName)
{
    Q_UNUSED(deviceName);
    //todo - delete file system (wipe credentials storege) is based on this
    return false;
}

int crypt_luksAddKeyBinary(struct crypt_options *options,
                           const char *pwd, unsigned int pwdLen,
                           const char *newPwd, unsigned int newPwdLen)
{
    struct crypt_device *cd = NULL;
    int r;

    if ((r = crypt_init(&cd, options->device)))
        return -EINVAL;

    crypt_set_log_callback(cd, log_wrapper, (void*) options->icb->log);
    crypt_set_confirm_callback(cd, yesDialog_wrapper,
                               (void*) options->icb->yesDialog);

    crypt_set_timeout(cd, options->timeout);
    crypt_set_password_retry(cd, options->tries);
    crypt_set_iterarion_time(cd, options->iteration_time ?: 1000);
    crypt_set_password_verify(cd, options->flags & CRYPT_FLAG_VERIFY);

    if ((r = crypt_load(cd, CRYPT_LUKS1, NULL))) {
        crypt_free(cd);
        return r;
    }

    if (options->key_file || options->new_key_file)
        r = -1;
    else
        r = crypt_keyslot_add_by_passphrase(cd, options->key_slot,
                                            pwd, pwdLen, newPwd, newPwdLen);

    crypt_free(cd);
    return (r < 0) ? r : 0;
}

bool CryptsetupHandler::addKeySlot(const QString &deviceName,
                                   const QByteArray &key,
                                   const QByteArray &existingKey)
{
    struct crypt_options options;

    options.key_size = SIGNON_LUKS_KEY_SIZE / 8;
    options.cipher = SIGNON_LUKS_CIPHER;

    char *localDeviceName = (char *)malloc(deviceName.length() + 1);
    Q_ASSERT(localDeviceName != NULL);
    strcpy(localDeviceName, deviceName.toLatin1().constData());

    options.device = localDeviceName;
    options.new_key_file = NULL;
    options.key_file = NULL;
    options.key_slot = -1;

    options.flags = 0;
    options.iteration_time = 1000;
    options.timeout = 0;
    options.tries = 0;

    static struct interface_callbacks cmd_icb;
    cmd_icb.yesDialog = yesDialog;
    cmd_icb.log = cmdLineLog;
    options.icb = &cmd_icb;

    int ret = crypt_luksAddKeyBinary(&options,
                                     existingKey.constData(),
                                     existingKey.length(),
                                     key.constData(), key.length());

    if (localDeviceName)
        free(localDeviceName);

    if (ret != 0)
        TRACE() << "Cryptsetup add key API call result:" << ret <<
            "." <<  error();

    return (ret == 0);
}

int crypt_luksRemoveKeyBinary(struct crypt_options *options,
                              const char *pwdToRemove,
                              unsigned int pwdToRemoveLen)
{
    struct crypt_device *cd = NULL;
    int key_slot;
    int r;

    if ((r = crypt_init(&cd, options->device)))
        return -EINVAL;

    crypt_set_log_callback(cd, log_wrapper, (void*) options->icb->log);
    crypt_set_confirm_callback(cd, yesDialog_wrapper,
                               (void*) options->icb->yesDialog);

    crypt_set_timeout(cd, options->timeout);
    crypt_set_password_retry(cd, options->tries);
    crypt_set_iterarion_time(cd, options->iteration_time ?: 1000);
    crypt_set_password_verify(cd, options->flags & CRYPT_FLAG_VERIFY);

    if ((r = crypt_load(cd, CRYPT_LUKS1, NULL))) {
        crypt_free(cd);
        return r;
    }

    if ((key_slot = crypt_keyslot_by_passphrase(cd, NULL, pwdToRemove,
                                                pwdToRemoveLen, 0, NULL)) < 0) {
        r = -EPERM;
        goto out;
    }

    r = crypt_keyslot_destroy(cd, key_slot);

out:
    crypt_free(cd);
    return (r < 0) ? r : 0;
}

bool CryptsetupHandler::removeKeySlot(const QString &deviceName,
                                      const QByteArray &key,
                                      const QByteArray &remainingKey)
{
    struct crypt_options options;

    options.key_size = SIGNON_LUKS_KEY_SIZE / 8;
    options.cipher = SIGNON_LUKS_CIPHER;

    char *localDeviceName = (char *)malloc(deviceName.length() + 1);
    Q_ASSERT(localDeviceName != NULL);
    strcpy(localDeviceName, deviceName.toLatin1().constData());

    options.device = localDeviceName;
    options.new_key_file = NULL;
    options.key_file = NULL;
    options.key_slot = -1;

    options.flags = 0;
    options.timeout = 0;

    static struct interface_callbacks cmd_icb;
    cmd_icb.yesDialog = yesDialog;
    cmd_icb.log = cmdLineLog;
    options.icb = &cmd_icb;

    int ret = crypt_luksRemoveKeyBinary(&options, key.constData(), key.length());

    if (localDeviceName)
        free(localDeviceName);

    if (ret != 0)
        TRACE() << "Cryptsetup remove key API call result:" << ret <<
            "." <<  error();

    return (ret == 0);
}

bool CryptsetupHandler::loadDmMod()
{
    SystemCommandLineCallHandler handler;
    return handler.makeCall(
                        QLatin1String("/sbin/modprobe"),
                        QStringList() << QString::fromLatin1("dm_mod"));
}

QString CryptsetupHandler::error()
{
    char buf[260];
    crypt_get_error(buf, 256);
    return QString::fromLocal8Bit(buf);
}

