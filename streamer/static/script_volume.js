const controlElem = parent.document.getElementById("control");
const currentElem = parent.document.getElementById("current");
let canSetVol = false;
const volumeTimeout = 100;
let volumeTimeId = setTimeout(function () { canSetVol = true }, volumeTimeout);

function showcontrol() {
    controlElem.hidden = false;
}

function updatecurrent() {
    currentElem.innerHTML = "volume";
}

function setlevel(volumeid) {
    if (canSetVol) {
        canSetVol = false;
        clearTimeout(volumeTimeId);
        inputElem = document.getElementById(volumeid);
        fetch("setvolume?" + volumeid + "&" + inputElem.value).then(resp => {
            if (resp.status != 200) {
                controlElem.hidden = true;
            }
        });
        volumeTimeId = setTimeout(function () { canSetVol = true }, volumeTimeout);
    }
}