const timeout = 3000;

function getalbums() {
    document.getElementById("albums").src = window.location.href + "albums";
}

function hidecontrol() {
    document.getElementById("volume").hidden = false;
    document.getElementById("control").hidden = true;
}

function getvolume() {
    fetch("getvolume")
        .then(resp => {
            if (resp.status == 200) {
                document.getElementById("volume").innerHTML = "&#9738";
                document.getElementById("control").min = resp.statusText.split("_")[0];
                document.getElementById("control").value = resp.statusText.split("_")[1];
                document.getElementById("control").max = resp.statusText.split("_")[2];
                document.getElementById("volume").hidden = true;
                document.getElementById("control").hidden = false;
                delayVar = setTimeout(hidecontrol, timeout);
            } else {
                document.getElementById("volume").innerHTML = "&#9739";
            }
        })
}
