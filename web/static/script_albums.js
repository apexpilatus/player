const iconElem = parent.document.getElementById("icon");

function gettracks(album) {
    if (iconElem.href != location.origin + album)
        parent.location.assign(location.origin + "/tracks?" + "album=" + album + "&scroll=" + pageYOffset);
}