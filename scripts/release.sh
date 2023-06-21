cd ..

# Verify you at least have all the commands to run the script, hopefully you have 'em configured too
if (( $(which sed | wc --chars) == 0 )) ; then
    echo "You are missing the sed command required to run this script."
    exit 1;
elif (( $(which platformio | wc --chars) == 0 )) ; then
    echo "You are missing the platformio command required to run this script."
    exit 1;
elif (( $(which git | wc --chars) == 0 )) ; then
    echo "You are missing the git command required to run this script."
    exit 1;
elif (( $(which gh | wc --chars) == 0 )) ; then
    echo "You are missing the gh command required to run this script."
    exit 1;
fi

# Update release version in config and readme
CONFIG_VERSION_STRING="static const std::string SOFTWARE_VERSION = \"v${1}\"; \/\/ SED-BUOY"
sed -i -e "s/^.*SED-BUOY.*$/$CONFIG_VERSION_STRING/" src/app/utilities/Config.h
README_VERSION_STRING="\[!\[release\ badge\ SED-BUOY]\(https:\/\/badgen\.net\/badge\/libros\/v${1}\/green\)]\(https:\/\/github\.com\/nvts8a\/libros\/raw\/main\/releases\/libros-LATEST\.uf2\)"
sed -i -e "s/^.*SED-BUOY.*$/$README_VERSION_STRING/" README.md

# Build and distribute new firmware
platformio run --environment pico
cp -n .pio/build/pico/firmware.uf2 releases/libros-$1.uf2
cp .pio/build/pico/firmware.uf2 releases/libros-LATEST.uf2

# Push to main, new tag, and release to Github
git add --all
git ci -m "[${1}] ${2}"
git push
git tag v${1}
git push origin v${1}
gh release create v${1} ./releases/libros-$1.uf2  --latest --title "v${1}" --notes "[${1}](https://github.com/nvts8a/libros/raw/main/releases/libros-${1}.uf2)"