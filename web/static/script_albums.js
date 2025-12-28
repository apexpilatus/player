function gettracks(album) {
        if (window.parent) {
                const iconElem = parent.document.getElementById("icon");
                if (iconElem.href != location.origin + album)
                        parent.location.assign(location.origin + "/tracks?" + "album=" + album + "&scroll=" + pageYOffset);
        }
}

function updatetop(album) {
        if (window.parent) {
                const topalbumElem = parent.document.getElementById("topalbum");
                topalbumElem.innerHTML = album;
        }
}