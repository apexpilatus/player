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
                location.assign(location.href);
            }
        }).catch(error => {
            location.assign(location.href);
        });
        volumeTimeId = setTimeout(function () { canSetVol = true }, volumeTimeout);
    }
}
