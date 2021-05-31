Name:           xpac
Version:        0.12
Release:        2
Summary:        xpac - a vintage clone of Pac-Man™
Group:          Games
License:        MIT
URL:            https://github.com/kamiljdudek/xpac
Vendor:         Kamil J. Dudek
Source:         xpac-0.12.tar.xz

Prefix:         %{_prefix}
Packager: 	Kamil J. Dudek
BuildRoot:      %{_tmppath}/%{name}-root

Requires:       xorg-x11-fonts-misc

%description
This is a basic version of pacman. I've tried to concentrate on the essentials
of the game for now, which means a lot of the original features are absent (ie
no scores, no fruit, no special items, no lives). When all the pills have been 
cleared from a level, the difficulty level is notched up, and a new level is
generated. If you collide with a ghost, all the pills are replaced, and you
restart the current level from scratch. A new feature just added is the
presence of large red pills that for a short time turn the ghosts grey and let
you eat them.

%global debug_package %{nil}

%prep
%setup -q -n %{name}-%{version}

%build
make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT/usr/bin/ MANDESTDIR=$RPM_BUILD_ROOT/usr/share/man/man6/ install
rm -rf $RPM_BUILD_ROOT%{_datadir}/doc/%{name}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
/usr/bin/xpac
/usr/share/man/man6/xpac.6.gz

%changelog


