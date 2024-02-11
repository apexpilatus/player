function gettracks(album) {
    parent.document.getElementById("albums").style.height = "30%";
    parent.document.getElementById("tracks").src = parent.window.location.href + "tracks?" + album;
}
