#!/bin/fish

# clear the old wine prefix and arch
set -e WINEPREFIX
set -e WINEARCH

# set the wine prefix and arch
set WINE_HOME (pwd)/.wineenv
set -gx WINEPREFIX $WINE_HOME
set -gx WINEARCH win64

# Add the msvc scripts to the path
set -e MSVC_PATH
set -gx MSVC_PATH $HOME/msvc/bin/x64

set msvc_in_path "n"
for loc in $PATH;
    if test $loc = $MSVC_PATH;
        set msvc_in_path "y"
        break
    end
end

if test $msvc_in_path = "n"
    fish_add_path -p -g $MSVC_PATH
end
