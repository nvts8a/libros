CONF=/media/steven/OPENBOOK/_OPENBOOK/openbook.conf
BAHD=/media/steven/OPENBOOK/BOOKS/harlan-ellison-a-boy-and-his-dog.txt
FRNK=/media/steven/OPENBOOK/BOOKS/mary-shelley-frankenstein.txt
if [ -f "$CONF" ]; then
    if [ -f "$BAHD" ]; then
        if [ -f "$FRNK" ]; then
            rm "$FRNK"
            rm "$BAHD"
            cp ../test/resources/short-stories/sunyi-dean-how-to-cook-and-eat-the-rich.txt /media/steven/OPENBOOK/
            cp ../test/resources/short-stories/vivianni-glass-synthetic-perennial.txt      /media/steven/OPENBOOK/BOOKS/
        else
            cp ../test/resources/novels/robert-evans-after-the-revolution.txt   /media/steven/OPENBOOK/
            cp ../test/resources/novels/mary-shelley-frankenstein.txt           /media/steven/OPENBOOK/BOOKS/
        fi
    else
        cp ../test/resources/short-stories/harlan-ellison-a-boy-and-his-dog.txt /media/steven/OPENBOOK/
        cp ../test/resources/short-stories/ian-r-macleod-the-chronologist.txt   /media/steven/OPENBOOK/

        cp ../test/resources/short-stories/neil-gaiman-i-cthulhu.txt            /media/steven/OPENBOOK/BOOKS/
        cp ../test/resources/short-stories/mary-anne-mohanraj-hush.txt          /media/steven/OPENBOOK/BOOKS/
        cp ../test/resources/short-stories/terry-pratchett-troll-bridge.txt     /media/steven/OPENBOOK/BOOKS/
    fi
else
    rm -rf /media/steven/OPENBOOK/*
    mkdir /media/steven/OPENBOOK/BOOKS
    mkdir /media/steven/OPENBOOK/_OPENBOOK
    touch "$CONF"

    cp ../test/resources/short-stories/anamaria-curtis-the-last-truth.txt   /media/steven/OPENBOOK/
    cp ../test/resources/short-stories/anjali-sachdeva-arbitrium.txt        /media/steven/OPENBOOK/

    cp ../test/resources/short-stories/grace-p-fong-girl-oil.txt            /media/steven/OPENBOOK/BOOKS/
    cp ../test/resources/short-stories/kemi-ashing-giwa-fruiting-bodies.txt /media/steven/OPENBOOK/BOOKS/
    cp ../test/resources/short-stories/kurt-vonnegut-2br02b.txt             /media/steven/OPENBOOK/BOOKS/
fi