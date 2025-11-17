const controlElem = parent.document.getElementById("control");
const playerElem = parent.document.getElementById("player");

function showtracks() {
    controlElem.className = "cdcontrol";
    controlElem.hidden = false;
}

function playcd(track) {
    fetch(location.origin + "/playcd?" + track).then(resp => {
        if (resp.status != 200)
            playerElem.src = location.origin + "/stream_cd?" + track;
    });
}