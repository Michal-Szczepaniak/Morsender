# 
# Do NOT Edit the Auto-generated Part!
# Generated by: spectacle version 0.27
# 

Name:       Morsender

# >> macros
# << macros

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}
Summary:    Morsender is multiprotocol messenger for sailfishos based on libpurple.
Version:    0.3.5
Release:    1
Group:      Qt/Qt
License:    GPL-3.0
URL:        http://example.org/
Source0:    %{name}-%{version}.tar.bz2
Source100:  Morsender.yaml
Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   libpurple
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  libpurple-devel
BuildRequires:  nemo-qml-plugin-notifications-qt5-devel
BuildRequires:  qt5-qtdbus-devel
BuildRequires:  libnemotransferengine-qt5-devel
BuildRequires:  desktop-file-utils

%description
Morsender is multiprotocol messenger for sailfishos based on libpurple.


%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
# << build pre

%qtc_qmake5 

%qtc_make %{?_smp_mflags}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake5_install

# >> install post
# << install post

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
/usr/lib/nemo-transferengine/plugins/libMorsenderSharePlugin.so
/usr/share/dbus-1/services/com.mistermagister.morsender.service
# >> files
# << files
