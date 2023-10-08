function gettrackspicture(album, rate) {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        element = parent.document.getElementById("trackspicture");
        element.src = "data:image/jpeg;base64," + this.responseText;
        element.setAttribute("onclick", "play(\"" + album + "\",\"" + rate + "\")");
        element.hidden = false;
        parent.document.getElementById("hidetracks").hidden = false;
        parent.document.getElementById("tracks").hidden = false;
    }
    xhttp.open("POST", parent.window.location.href + "album?album=" + album.replace(/&/g, " "));
    xhttp.send();
}

function play(album, rate, track) {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
    }
    xhttp.open("GET", parent.window.location.href + "play?album=" + album.replace(/&/g, " ") + "&rate=" + rate + "&track=" + track);
    xhttp.send();
}