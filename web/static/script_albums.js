const iconElem = parent.document.getElementById("icon");
const topalbumElem = parent.document.getElementById("topalbum");

function gettracks(album) {
        if (iconElem != null) {
                if (iconElem.href != location.origin + album)
                        parent.location.assign(location.origin + "/tracks?" + "album=" + album + "&scroll=" + pageYOffset);
        } else
                fetch("http://store/playflac?" + album).then(resp => {
                        if (resp.status == 200)
                                location.assign(location.origin + "/albums?scroll=0");
                });
}

function updatetop(album) {
        if (topalbumElem != null)
                topalbumElem.innerHTML = album;
}
