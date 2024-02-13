function gettracks(album) {
    parent.document.getElementById("tracks").hidden = true;
    parent.document.getElementById("tracks").src = parent.window.location.href + "tracks?" + album;
}
