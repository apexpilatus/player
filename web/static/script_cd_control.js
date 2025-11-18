const controlElem = parent.document.getElementById("control");
const playerElem = parent.document.getElementById("player");
const iconElem = parent.document.getElementById("icon");

function showtracks() {
    controlElem.className = "cdcontrol";
    controlElem.hidden = false;
}

function playcd(track) {
    fetch(location.origin + "/playcd?" + track).then(resp => {
        if (resp.status != 200) {
            if (iconElem.title != "cd")
                parent.location = location.origin + "/inbrowser?" + track
            playerElem.src = location.origin + "/stream_cd?" + track;
		playerElem.play();
        } else if (iconElem.title != "default" && iconElem.title != "null")
            parent.location = location.origin + "/inbrowser"
    });
}
