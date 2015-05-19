Name: signon-qt5
Version: 8.57
Release: 4
Summary: Single Sign On framework
Group: System/Libraries
License: LGPLv2.1
URL: https://code.google.com/p/accounts-sso.signond/
Source0: %{name}-%{version}.tar.bz2
BuildRequires: doxygen
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Sql)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(libcryptsetup)
BuildRequires: pkgconfig(accounts-qt5)
BuildRequires: pkgconfig(libproxy-1.0)
BuildRequires: pkgconfig(qt5-boostable)
BuildRequires: fdupes
BuildRequires: oneshot
Requires: mapplauncherd-qt5
Requires: oneshot
%{_oneshot_requires_post}
Obsoletes: signon

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_libdir}/libsignon-extension.so.*
%{_libdir}/libsignon-plugins-common.so.*
%{_libdir}/libsignon-plugins.so.*
%{_datadir}/dbus-1/services/*
%config %{_sysconfdir}/signond.conf
%{_libdir}/signon/libpasswordplugin.so
%{_oneshotdir}/signon-storage-perm

%package -n libsignon-qt5
Summary: Single Sign On Qt library
Group: System/Libraries
Requires: %{name} = %{version}-%{release}

%description -n libsignon-qt5
%{summary}

%files -n libsignon-qt5
%defattr(-,root,root,-)
%{_libdir}/libsignon-qt5.so.*

%post -n libsignon-qt5 -p /sbin/ldconfig
%postun -n libsignon-qt5 -p /sbin/ldconfig

%package testplugin
Summary: Single Sign On test plugins
Group: System/Libraries
Requires: %{name} = %{version}-%{release}
Obsoletes: signon-testplugin

%description testplugin
%{summary}

%files testplugin
%defattr(-,root,root,-)
%{_libdir}/signon/libssotest*.so


%package exampleplugin
Summary: Single Sign On example client
Group: System/Libraries
Requires: %{name} = %{version}-%{release}
Obsoletes: signon-exampleplugin

%description exampleplugin
%{summary}

%files exampleplugin
%defattr(-,root,root,-)
%{_libdir}/signon/libexampleplugin.so


%package devel
Summary: Development files for signon
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Obsoletes: signon-devel

%description devel
%{summary}

%files devel
%defattr(-,root,root,-)
%{_includedir}/signond/*
%{_includedir}/signon-extension/*
%{_includedir}/signon-plugins/*
%{_libdir}/libsignon-extension.so
%{_libdir}/libsignon-plugins-common.so
%{_libdir}/libsignon-plugins.so
%{_libdir}/pkgconfig/signond.pc
%{_libdir}/pkgconfig/signon-plugins.pc
%{_libdir}/pkgconfig/signon-plugins-common.pc
%{_libdir}/pkgconfig/SignOnExtension.pc
%{_libdir}/cmake/SignOnQt5/SignOnQt5Config.cmake
%{_libdir}/cmake/SignOnQt5/SignOnQt5ConfigVersion.cmake
%{_datadir}/dbus-1/interfaces/*


%package -n libsignon-qt5-devel
Summary: Development files for libsignon-qt
Group: Development/Libraries
Requires: libsignon-qt5 = %{version}-%{release}

%description -n libsignon-qt5-devel
%{summary}

%files -n libsignon-qt5-devel
%defattr(-,root,root,-)
%{_includedir}/signon-qt5/*
%{_libdir}/libsignon-qt5.so
%exclude %{_libdir}/libsignon-qt5.a
%{_libdir}/pkgconfig/libsignon-qt5.pc


%package doc
Summary: Documentation for signon
Group: Documentation
Obsoletes: signon-doc

%description doc
Doxygen-generated HTML documentation for the signon.

%files doc
%defattr(-,root,root,-)
%{_docdir}/signon/*
%{_docdir}/signon-plugins-dev/*
%{_docdir}/signon-plugins/*


%package -n libsignon-qt5-doc
Summary: Documentation for signon-qt
Group: Documentation

%description -n libsignon-qt5-doc
Doxygen-generated HTML documentation for the signon-qt

%files -n libsignon-qt5-doc
%defattr(-,root,root,-)
%{_docdir}/libsignon-qt5/*


%package tests
Summary: Tests for signon
Group: System/X11
Requires: %{name} = %{version}-%{release}
Requires: %{name}-testplugin = %{version}-%{release}
Obsoletes: signon-tests

%description tests
This package contains tests for signon

%files tests
%defattr(-,root,root,-)
/opt/tests/signon


%prep
%setup -q -n %{name}-%{version}/libsignon

chmod +x tests/create-tests-definition.sh

%build
%qmake5 TESTDIR=/opt/tests/signon CONFIG+=install_tests CONFIG+=enable-p2p
make %{?_smp_mflags}


%install
%qmake5_install
rm -f %{buildroot}/%{_docdir}/libsignon-qt/html/installdox
rm -f %{buildroot}/%{_docdir}/signon/html/installdox
rm -f %{buildroot}/%{_docdir}/signon-plugins/html/installdox
rm -f %{buildroot}/%{_docdir}/saslplugin/html/installdox
%fdupes %{buildroot}/%{_docdir}

mkdir -p %{buildroot}/%{_oneshotdir}
install -D -m 755 oneshot/signon-storage-perm %{buildroot}/%{_oneshotdir}

%post
/sbin/ldconfig
%{_bindir}/add-oneshot signon-storage-perm

%postun -p /sbin/ldconfig
