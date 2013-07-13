Name: signon
Version: 8.50
Release: 2
Summary: Single Sign On framework
Group: System/Libraries
License: LGPLv2.1
URL: https://code.google.com/p/accounts-sso.signond/
Source: %{name}-%{version}.tar.bz2
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

Patch0: %{name}-%{version}-install-tests.patch
Patch1: 0001-libsignon-disable-multilib.patch
Patch2: 0002-libsignon-c++0x.patch
Patch3: 0003-libsignon-documentation-path.patch
Patch4: 0004-Convert-QDBusArgument-session-parameters-to-QVariant.patch

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_libdir}/libsignon-extension.so.*
%{_libdir}/libsignon-plugins-common.so.*
%{_datadir}/dbus-1/services/*
%config %{_sysconfdir}/signond.conf
%{_libdir}/signon/libpasswordplugin.so

%package -n libsignon-qt
Summary: Single Sign On Qt library
Group: System/Libraries
Requires: %{name} = %{version}-%{release}

%description -n libsignon-qt
%{summary}

%files -n libsignon-qt
%defattr(-,root,root,-)
%{_libdir}/libsignon-qt.so.*

%post -n libsignon-qt -p /sbin/ldconfig
%postun -n libsignon-qt -p /sbin/ldconfig

%package testplugin
Summary: Single Sign On test plugins
Group: System/Libraries
Requires: %{name} = %{version}-%{release}
Provides: libsignon-testplugin = %{version}-%{release}
Obsoletes: libsignon-testplugin < %{version}-%{release}

%description testplugin
%{summary}

%files testplugin
%defattr(-,root,root,-)
%{_libdir}/%{name}/libssotest*.so


%package exampleplugin
Summary: Single Sign On example client
Group: System/Libraries
Requires: %{name} = %{version}-%{release}
Provides: libsignon-exampleplugin = %{version}-%{release}
Obsoletes: libsignon-exampleplugin < %{version}-%{release}

%description exampleplugin
%{summary}

%files exampleplugin
%defattr(-,root,root,-)
%{_libdir}/%{name}/libexampleplugin.so


%package devel
Summary: Development files for signon
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Provides: libsignon-devel = %{version}-%{release}
Obsoletes: libsignon-devel < %{version}-%{release}

%description devel
%{summary}

%files devel
%defattr(-,root,root,-)
%{_includedir}/signond/*
%{_includedir}/signon-extension/*
%{_includedir}/signon-plugins/*
%{_libdir}/libsignon-extension.so
%{_libdir}/libsignon-plugins-common.so
%{_libdir}/libsignon-plugins.a
%{_libdir}/pkgconfig/signond.pc
%{_libdir}/pkgconfig/signon-plugins.pc
%{_libdir}/pkgconfig/signon-plugins-common.pc
%{_libdir}/pkgconfig/SignOnExtension.pc
%{_datadir}/dbus-1/interfaces/*


%package -n libsignon-qt-devel
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
Summary: Documentation for signon
Group: Documentation
Provides: libsignon-doc = %{version}-%{release}
Obsoletes: libsignon-doc < %{version}-%{release}

%description doc
Doxygen-generated HTML documentation for the signon.

%files doc
%defattr(-,root,root,-)
%{_docdir}/signon/*
%{_docdir}/signon-plugins-dev/*
%{_docdir}/signon-plugins/*


%package -n libsignon-qt-doc
Summary: Documentation for signon-qt
Group: Documentation

%description -n libsignon-qt-doc
Doxygen-generated HTML documentation for the signon-qt

%files -n libsignon-qt-doc
%defattr(-,root,root,-)
%{_docdir}/libsignon-qt/*


%package tests
Summary: Tests for signon
Group: System/X11
Requires: %{name} = %{version}-%{release}
Provides: libsignon-tests = %{version}-%{release}
Obsoletes: libsignon-tests < %{version}-%{release}

%description tests
This package contains tests for signon

%files tests
%defattr(-,root,root,-)
/opt/tests/%{name}


%prep
%setup -n %{name}-%{version}
%patch0 -p1
%patch1 -p1
%patch2 -p1
%patch3 -p1
%patch4 -p1

chmod +x tests/create-tests-definition.sh

%build
qmake %{name}.pro TESTDIR=/opt/tests/%{name} CONFIG+=install_tests
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
