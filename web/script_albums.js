let current = "empty";

function gettracks(album) {
    if (current == album) {
        fetch("play?" + album)
    } else {
        current = album.slice();
        parent.document.getElementById("tracks").hidden = true;
        parent.document.getElementById("tracks").src = parent.window.location.href + "tracks?" + album;
    }
}
