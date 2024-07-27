const timeout = 3000;
let timeId = null;

function getalbums() {
    document.getElementById("albums").src = window.location.href + "albums";
}

function hidecontrol() {
    document.getElementById("volume").hidden = false;
    document.getElementById("control").hidden = true;
}

function getvolume() {
    fetch("getvolume").then(resp => {
        if (resp.status == 200) {
            if (timeId != null) {
                clearTimeout(timeId);
            }
            document.getElementById("volume").innerHTML = "&#9738";
            document.getElementById("volume").hidden = true;
            document.getElementById("control").hidden = false;
            document.getElementById("control").max = resp.statusText.split("_")[2];
            document.getElementById("control").min = resp.statusText.split("_")[0];
            document.getElementById("control").value = resp.statusText.split("_")[1];
            timeId = setTimeout(hidecontrol, timeout);
        } else {
            document.getElementById("volume").innerHTML = "&#9739";
        }
    })
}

function setvolume() {
    fetch("setvolume&" + document.getElementById("control").value).then(resp => {
        if (resp.status == 200) {
            clearTimeout(timeId);
            timeId = setTimeout(hidecontrol, timeout);
        } else {
            hidecontrol();
            document.getElementById("volume").innerHTML = "&#9739";
        }
    })
}
