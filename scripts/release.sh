# Update release version in config and readme
CONFIG_VERSION_STRING="        return \"v${1}\"; \/\/ SED-BUOY"
sed -i -e "s/^.*SED-BUOY.*$/$CONFIG_VERSION_STRING/" ../src/app/utilities/Config.h
README_VERSION_STRING="\[!\[release\ badge\ SED-BUOY]\(https:\/\/badgen\.net\/badge\/libros\/v${1}\/green\)]\(https:\/\/github\.com\/nvts8a\/libros\/raw\/main\/releases\/libros-LATEST\.uf2\)"
sed -i -e "s/^.*SED-BUOY.*$/$README_VERSION_STRING/" ../README.md

# Build and distribute new firmware
platformio run --environment pico
cp -n ../.pio/build/pico/firmware.uf2 ../releases/libros-$1.uf2
cp ../.pio/build/pico/firmware.uf2 ../releases/libros-LATEST.uf2

# Push to main and new tag to Github
git add --all
git ci -m "[${1}] ${2}"
git push
git tag v${1}
git push origin v${1}