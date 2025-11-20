const iconElem = parent.document.getElementById("icon");

function gettracks(album) {
    alert(iconElem.href + " - " + album);
    if (iconElem.href != album)
        parent.location.assign(location.origin + "/tracks?" + "album=" + album + "&scroll=" + pageYOffset);
}