function gettrackspicture(album) {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        element = parent.document.getElementById("trackspicture");
        element.src = "data:image/jpeg;base64," + this.responseText;
        element.setAttribute("onclick", "play(\"" + album + "\")");
    }
    xhttp.open("POST", parent.window.location.href + "album?album=" + album.replace(/&/g, " "));
    xhttp.send();
}

function play(album, track) {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        parent.document.getElementById("trackspicture").setAttribute("class", "destroyed");
    }
    xhttp.open("GET", parent.window.location.href + "play?album=" + album.replace(/&/g, " ") + "&track=" + track);
    xhttp.send();
}