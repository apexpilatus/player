const controlElem = parent.document.getElementById("control");
const scrollupElem = parent.document.getElementById("scrollup");
const scrolldownElem = parent.document.getElementById("scrolldown");
const currentElem = parent.document.getElementById("current");
const topElem = parent.document.getElementById("top");
const playerElem = parent.document.getElementById("player");
let empty = false;

function updatetop(album) {
    topElem.innerHTML = album;
}

function gettracks(album) {
    if (currentElem.innerHTML == album) {
        fetch("playflac?" + album).then(resp => {
            if (resp.status != 200)
                //playerElem.src = "stream_album?" + album;
                parent.window.location.assign("/?" + album);
            else if (topElem.innerHTML != album)
                window.location.assign("albums?up");
        });
    } else {
        controlElem.hidden = true;
        controlElem.src = "tracks?" + album;
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
