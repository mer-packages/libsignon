Name: libsignon-qt
Version: 8.50
Release: 2
Summary: Single Sign On Qt library
Group: System/Libraries
License: LGPLv2.1
URL: https://code.google.com/p/accounts-sso.signond/
Source0: %{name}-%{version}.tar.bz2
BuildRequires: doxygen
BuildRequires: pkgconfig(QtCore)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(libcryptsetup)
BuildRequires: pkgconfig(accounts-qt)
BuildRequires: pkgconfig(libproxy-1.0)
BuildRequires: fdupes

Provides: libsignon-passwordplugin = %{version}-%{release}
Obsoletes: libsignon-passwordplugin < %{version}-%{release}
Provides: libsignon = %{version}-%{release}
Obsoletes: libsignon < %{version}-%{release}

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_libdir}/libsignon-qt.so.*
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
%exclude /opt/tests/signon

%package devel
Summary: Development files for libsignon-qt
Group: Development/Libraries
Requires: libsignon-qt = %{version}-%{release}

%description -n libsignon-qt-devel
%{summary}

%files -n libsignon-qt-devel
%defattr(-,root,root,-)
%{_includedir}/signon-qt/*
%{_libdir}/libsignon-qt.so
%exclude %{_libdir}/libsignon-qt.a
%{_libdir}/pkgconfig/libsignon-qt.pc


%package doc
Summary: Documentation for signon-qt
Group: Documentation
Provides: libsignon-doc = %{version}-%{release}
Obsoletes: libsignon-doc < %{version}-%{release}

%description doc
%{summary}.

%files doc
%defattr(-,root,root,-)
%{_docdir}/libsignon-qt/*

%prep
%setup -q -n %{name}-%{version}/libsignon

chmod +x tests/create-tests-definition.sh

%build
%qmake TESTDIR=/opt/tests/signon CONFIG+=install_tests
make %{?jobs:-j%jobs}


%install
%qmake_install
rm -f %{buildroot}/%{_docdir}/libsignon-qt/html/installdox
rm -f %{buildroot}/%{_docdir}/signon/html/installdox
rm -f %{buildroot}/%{_docdir}/signon-plugins/html/installdox
rm -f %{buildroot}/%{_docdir}/saslplugin/html/installdox
%fdupes %{buildroot}/%{_docdir}


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

