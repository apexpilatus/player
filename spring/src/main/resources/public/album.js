function gettrackspicture(album) {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        element = parent.document.getElementById("trackspicture");
        element.src = "data:image/jpeg;base64," + this.responseText;
        element.setAttribute("onclick", "play(\"" + album + "\")");
        element.hidden = false;
        parent.document.getElementById("hidetracks").hidden = false;
        parent.document.getElementById("tracks").hidden = false;
    }
    xhttp.open("POST", parent.window.location.href + "album?album=" + album.replace(/&/g, " "));
    xhttp.send();
}

function play(album, track) {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        parent.document.getElementById("trackspicture").hidden = true;
    }
    xhttp.open("GET", parent.window.location.href + "play?album=" + album.replace(/&/g, " ") + "&track=" + track);
    xhttp.send();
}

function getbattery() {
    const xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        document.getElementById("battery").innerHTML = "&#9889;" + this.responseText;
    }
    window.setInterval(function () {
        xhttp.open("GET", parent.window.location.href + "battery");
        xhttp.send();
    }, 10000)
}