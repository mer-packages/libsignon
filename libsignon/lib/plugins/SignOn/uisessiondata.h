/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 *
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
/*!
 * @copyright Copyright (C) 2009-2011 Nokia Corporation.
 * @license LGPL
 */

#ifndef UISESSIONDATA_H
#define UISESSIONDATA_H

#include <SignOn/SessionData>
/*!
 * Error codes for ui interaction.
 */

namespace SignOn {

enum QueryError {
    QUERY_ERROR_NONE = 0,        /**< No errors. */
    QUERY_ERROR_GENERAL,         /**< Generic error during interaction. */
    QUERY_ERROR_NO_SIGNONUI,     /**< Cannot send request to signon-ui. */
    QUERY_ERROR_BAD_PARAMETERS,  /**< Signon-Ui cannot create dialog based on
                                   the given UiSessionData. */
    QUERY_ERROR_CANCELED,        /**< User canceled action. Plugin should not
                                   retry automatically after this. */
    QUERY_ERROR_NOT_AVAILABLE,   /**< Requested ui is not available. For
                                   example browser cannot be started. */
    QUERY_ERROR_BAD_URL,         /**< Given url was not valid. */
    QUERY_ERROR_BAD_CAPTCHA,     /**< Given captcha image was not valid. */
    QUERY_ERROR_BAD_CAPTCHA_URL, /**< Given url for capctha loading was not
                                   valid. */
    QUERY_ERROR_REFRESH_FAILED,  /**< Refresh failed. */
    QUERY_ERROR_FORBIDDEN,       /**< Showing ui forbidden by ui policy. */
    QUERY_ERROR_FORGOT_PASSWORD  /**< User pressed forgot password. */
    //TODO add more errors
};

/*!
 * Predefined messages to be shown to user while querying input.
 */
enum QueryMessageId {
    QUERY_MESSAGE_EMPTY = 0,          /**< No predefined message. */
    QUERY_MESSAGE_LOGIN,     /**< Login without prior errors. */
    QUERY_MESSAGE_NOT_AUTHORIZED          /**< Authentication failed. */
    //TODO add more messages
};

/*!
 * @class UiSessionData
 * Data container to hold values for authentication session.
 * Inherit this class if you want to extend the property range.
 *
 * @warning All this class' definitions must be inline.
 */
class UiSessionData : public SessionData
{
public:
    /*!
     * Constructor. Creates a UiSessionData with data 'data'.
     * @param data The data to be contained by the UiSessionData
     * @attention internal use only recommended. As a SSO client application
     * developer use setters/gettters for specific SessionData properties.
     */
    UiSessionData(const QVariantMap &data = QVariantMap()) { m_data = data; }

    /*!
     * Declares the property QueryError setter and getter.
     * QueryError is used to report errors in ui interaction
     * that is shown to user.
     * @see QueryError
     */
    SIGNON_SESSION_DECLARE_PROPERTY(int, QueryErrorCode)

    /*!
     * Declare the property Caption setter and getter.
     * Caption is used to set credentials caption.
     * Empty caption is discarded.
     * @warning This string is shown to user as it is,
     * plugin is responsible for localization.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, Caption)

    /*!
     * Declare the property Title setter and getter.
     * Title is used to set dialog title for password confirmation.
     * @warning This string is shown to user as it is.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, Title)

    /*!
     * Declares the property QueryMessageId setter and getter.
     * QueryMessageId is used to select predefined localized message
     * that is shown to user.
     * @see QueryMessageId
     */
    SIGNON_SESSION_DECLARE_PROPERTY(int, QueryMessageId)

    /*!
     * Declares the property QueryMessage setter and getter.
     * QueryMessage is used to show given message to user.
     * Empty message is discarded.
     * @warning This string is shown to user as it is,
     * plugin is responsible for localization.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, QueryMessage)

    /*!
     * Declares the property QueryUserName setter and getter.
     * QueryUserName is used to enable username input in signon-ui.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(bool, QueryUserName)

    /*!
     * Declares the property QueryPassword setter and getter.
     * QueryPassword is used to enable password input in signon-ui.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(bool, QueryPassword)

    /*!
     * Declares the property RememberPassword setter and getter.
     * RememberPassword is used to enable password storing signond.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(bool, RememberPassword)

    /*!
     * Declares the property ShowRealm setter and getter.
     * ShowRealm is used to show realm in signon-ui.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(bool, ShowRealm)

    /*!
     * Declares the property OpenUrl setter and getter.
     * This url is passed to signon-ui to be opened and shown to user.
     * Empty url does not open the url view.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, OpenUrl)

    /*!
     * Declares the property FinalUrl setter and getter.
     * If this URL is set, the signon-ui browser widget will close
     * automatically when this URL is reached.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, FinalUrl)

    /*!
     * Declares the property UrlResponse setter and getter.
     * Response from signon-ui for OpenUrl request.
     * After completion UrlResponse contains redirect target url, if there was
     * redirection.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, UrlResponse)

    /*!
     * Declares the property CaptchaUrl setter and getter.
     * CaptchaUrl contains url to the captcha image to be verified by user.
     * CaptchaUrl is used in signon-ui when plugin requires the captcha
     * verification from user.
     * If this is empty, captcha query is not shown, unless CaptchaImage
     * contain valid image.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, CaptchaUrl)

    /*!
     * Declares the property CaptchaImage setter and getter.
     * CaptchaImage contains the captcha image to be verified by user.
     * CaptchaImage is passed to signon-ui when plugin requires the captcha
     * verification from user.
     * If this is empty, captcha query is not shown, unless CaptchaUrl point to
     * valid image.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QByteArray, CaptchaImage)

    /*!
     * Declares the property CaptchaResponse setter and getter
     * CaptchaResponse is passed to plugin after signon-ui has processed the
     * captcha query.
     * It contains user input for captcha query.
     * @see CaptchaUrl
     * @see CaptchaImage.
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, CaptchaResponse)

    /*!
     * Declares the property ForgotPassword setter and getter
     * ForgotPassword string is shown as a link to user.
     * @see ForgotPasswordUrl
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, ForgotPassword)

    /*!
     * Declares the property ForgotPasswordUrl setter and getter
     * ForgotPasswordUrl is shown to user if ForgotPassword link is pressed.
     * @see ForgotPassword
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, ForgotPasswordUrl)

    /*!
     * Declares the property Confirm setter and getter
     * This will set dialog to confirm password mode.
     * @see ForgotPasswordUrl
     */
    SIGNON_SESSION_DECLARE_PROPERTY(bool, Confirm)

    /*!
     * Declares the property Icon setter and getter
     * Icon is shown to user in signin dialog.
     * @see ForgotPassword
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, Icon)

};

} //namespace SignOn

Q_DECLARE_METATYPE(SignOn::UiSessionData)
#endif // UISESSIONDATA_H
