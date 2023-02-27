package controllers;

import beans.Storage;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;
import java.util.List;
import java.util.Map;

@RestController
public class MainPage {
    @GetMapping("/")
    void mainPage(@RequestHeader("Host") String host, HttpServletResponse resp, Storage store) throws IOException {
        Map<String, List<String>> albums = store.getAlbums();
        resp.setContentType("text/html");
        resp.getWriter().println("<head><meta charset=UTF-8><title>player</title>");
        resp.getWriter().println("<link rel=apple-touch-icon href=apple-180x180.png sizes=180x180 type=image/png>");
        resp.getWriter().println("<script>");
        resp.getWriter().println("function setVolume(direction){");
        resp.getWriter().println("\tconst xhttp = new XMLHttpRequest();");
        resp.getWriter().println("\txhttp.onload = function() {");
        resp.getWriter().println("\t\tdocument.getElementById(\"volume\").innerHTML = \"&#127911 \" + this.responseText;");
        resp.getWriter().println("\t}");
        resp.getWriter().println("\txhttp.open(\"GET\", window.location.href + \"volume\" + \"?direction=\" + direction);");
        resp.getWriter().println("\txhttp.send();");
        resp.getWriter().println("}");
        resp.getWriter().println("function getTracks(album){");
        resp.getWriter().println("\tdocument.getElementById(\"tracks\").src = \"http://" + host + "/album?album=\" + album.replace(/&/g, \" \")");
        resp.getWriter().println("\tdocument.getElementById(\"tracks\").hidden = false;");
        resp.getWriter().println("\tdocument.getElementById(\"trackspicture\").hidden = false;");
        resp.getWriter().println("\tdocument.getElementById(\"hidetracks\").hidden = false;");
        resp.getWriter().println("}");
        resp.getWriter().println("function hideTracks(){");
        resp.getWriter().println("\tdocument.getElementById(\"tracks\").hidden = true;");
        resp.getWriter().println("\tdocument.getElementById(\"trackspicture\").hidden = true;");
        resp.getWriter().println("\tdocument.getElementById(\"hidetracks\").hidden = true;");
        resp.getWriter().println("}");
        resp.getWriter().println("</script>");
        resp.getWriter().println("</head>");
        resp.getWriter().println("<body style=background-color:slategray>");
        resp.getWriter().println("<p id=volume style=position:fixed;top:80px;left:20px;font-size:20px;></p>");
        resp.getWriter().println("<button type=button onclick=setVolume(\"up\") style=border-radius:20px;color:black;background-color:white;font-size:20px;position:fixed;top:50px;left:20px;>up</button>");
        resp.getWriter().println("<button type=button onclick=setVolume(\"down\") style=border-radius:20px;color:black;background-color:white;font-size:20px;position:fixed;top:150px;left:20px;>dw</button>");
        resp.getWriter().println("<script>setVolume(\"init\")</script>");
        resp.getWriter().println("<iframe hidden id=tracks width=450 height=430 style=\"position:fixed;top:10px;right:10px;border-style:solid;\"></iframe>");
        resp.getWriter().println("<img hidden id=trackspicture style=width:115px;height:115px;position:fixed;top:10px;right:480px;border-style:solid;>");
        resp.getWriter().println("<button hidden type=button id=hidetracks onclick=hideTracks() style=border-radius:20%;color:red;background-color:black;font-size:25px;position:fixed;top:460px;right:10px;>X</button>");
        resp.getWriter().println("<ul style=padding-left:271px;font-size:150%;line-height:180%;list-style-type:circle;>");
        albums.forEach((album, albumPathList) -> albumPathList.forEach((albumPath) -> {
            try {
                resp.getWriter().println("<li><b style=color:black; onclick=getTracks(\"" + (albumPath + "/" + album).replace(" ", "&") + "\")>" + album.replace("fuckingslash", "/").replace("fuckingblackstar", "&#9733").replace("fuckingplus", "&#43").replace(" anD ", " & ").replace("___", " <small style=color:white;>") + "</li></b></small>");
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }));
        resp.getWriter().println("</ul>");
        resp.getWriter().println("</body>");
    }
}