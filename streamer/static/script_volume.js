let canSetVol = false;
const volumeTimeout = 100;
let volumeTimeId = setTimeout(function () { canSetVol = true }, volumeTimeout);

function setlevel(volumeid) {
    if (canSetVol) {
        canSetVol = false;
        clearTimeout(volumeTimeId);
        inputElem = document.getElementById(volumeid);
        fetch("setvolume?" + volumeid + "&" + inputElem.value).then(resp => {
            if (resp.status != 200) {
                window.location.assign(window.location.href);
            }
        }).catch(error => {
            window.location.assign(window.location.href);
        });
        volumeTimeId = setTimeout(function () { canSetVol = true }, volumeTimeout);
    }
}