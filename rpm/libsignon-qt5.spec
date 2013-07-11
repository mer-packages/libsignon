Name: libsignon-qt5
Version: 8.50
Release: 2
Summary: Single Sign On Qt5 library
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
BuildRequires: fdupes

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_libdir}/libsignon-qt5.so.*
%exclude %{_bindir}/*
%exclude %{_libdir}/libsignon-extension.so.*
%exclude %{_libdir}/libsignon-plugins-common.so.*
%exclude %{_datadir}/dbus-1/services/*
%exclude %{_sysconfdir}/signond.conf
%exclude %{_libdir}/signon/libpasswordplugin.so
%exclude %{_libdir}/signon/libssotest*.so
%exclude %{_libdir}/signon/libexampleplugin.so
%exclude %{_includedir}/signond/*
%exclude %{_includedir}/signon-extension/*
%exclude %{_includedir}/signon-plugins/*
%exclude %{_libdir}/libsignon-extension.so
%exclude %{_libdir}/libsignon-plugins-common.so
%exclude %{_libdir}/libsignon-plugins.a
%exclude %{_libdir}/pkgconfig/signond.pc
%exclude %{_libdir}/pkgconfig/signon-plugins.pc
%exclude %{_libdir}/pkgconfig/signon-plugins-common.pc
%exclude %{_libdir}/pkgconfig/SignOnExtension.pc
%exclude %{_datadir}/dbus-1/interfaces/*
%exclude %{_docdir}/signon/*
%exclude %{_docdir}/signon-plugins-dev/*
%exclude %{_docdir}/signon-plugins/*
%exclude %{_libdir}/debug/*
%exclude %{_libdir}/debug/.build-id/*
%exclude /opt/tests/signon

%package devel
Summary: Development files for libsignon-qt5
Group: Development/Libraries
Requires: libsignon-qt5 = %{version}-%{release}

%description devel
%{summary}

%files devel
%defattr(-,root,root,-)
%{_includedir}/signon-qt5/*
%{_libdir}/libsignon-qt5.so
%exclude %{_libdir}/libsignon-qt5.a
%{_libdir}/pkgconfig/libsignon-qt5.pc


%package doc
Summary: Documentation for signon-qt5
Group: Documentation

%description doc
Doxygen-generated HTML documentation for libsignon-qt5

%files doc
%defattr(-,root,root,-)
%{_docdir}/libsignon-qt5/*


%prep
%setup -q -n %{name}-%{version}/libsignon

chmod +x tests/create-tests-definition.sh

%build
%qmake5 signon.pro TESTDIR=/opt/tests/signon CONFIG+=install_tests
make


%install
make INSTALL_ROOT=%{buildroot} install
rm -f %{buildroot}/%{_docdir}/libsignon-qt/html/installdox
rm -f %{buildroot}/%{_docdir}/signon/html/installdox
rm -f %{buildroot}/%{_docdir}/signon-plugins/html/installdox
rm -f %{buildroot}/%{_docdir}/saslplugin/html/installdox
%fdupes %{buildroot}/%{_docdir}


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
