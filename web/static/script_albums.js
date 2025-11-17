const controlElem = parent.document.getElementById("control");
const scrollupElem = parent.document.getElementById("scrollup");
const scrolldownElem = parent.document.getElementById("scrolldown");
const selectedalbumElem = parent.document.getElementById("selectedalbum");
const topalbumElem = parent.document.getElementById("topalbum");
const playerElem = parent.document.getElementById("player");
let empty = false;

function updatetop(album) {
    topalbumElem.innerHTML = album;
}

function gettracks(album) {
    alert(location.href + " - " + location.origin)
    if (selectedalbumElem.innerHTML == album) {
        fetch(parent.window.location.href + "playflac?" + album).then(resp => {
            if (resp.status != 200)
                playerElem.src = parent.window.location.href + "stream_album?" + album;
            else if (topalbumElem.innerHTML != album)
                window.location.assign(parent.window.location.href + "albums?up");
        });
    } else {
        controlElem.hidden = true;
        controlElem.src = parent.window.location.href + "tracks?" + album;
    }
}

function hidescroll() {
    empty = true;
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
}

function showscroll(initscroll) {
    if (!empty) {
        if (initscroll == "down")
            window.scrollTo(0, document.body.scrollHeight);
        scrollupElem.hidden = false;
        scrolldownElem.hidden = false;
    }
}