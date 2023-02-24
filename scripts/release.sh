cp -n ../.pio/build/pico/firmware.uf2 ../releases/libros-$1.uf2
cp ../.pio/build/pico/firmware.uf2 ../releases/libros-LATEST.uf2
git add --all
git ci -m "[${1}] ${2}"
git push

git tag v${1}
git push v${1}