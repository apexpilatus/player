package controllers;

import beans.Storage;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.List;
import java.util.Map;

@RestController
public class MainPage {
    @GetMapping("/")
    void mainPage(HttpServletResponse resp, Storage store) throws IOException {
        Map<String, List<String>> albums = store.getAlbums();
        resp.setContentType("text/html");
        resp.setCharacterEncoding("utf-8");
        resp.setHeader("Cache-Control", "no-cache");
        resp.setHeader("X-Content-Type-Options", "nosniff");
        PrintWriter respWriter = resp.getWriter();
        respWriter.println("<!DOCTYPE html>");
        respWriter.println("<html lang=en>");
        respWriter.println("<head><meta name=viewport content=\"width=device-width, initial-scale=1.0\" charset=utf-8>");
        respWriter.println("<title>player</title>");
        respWriter.println("<link rel=apple-touch-icon href=apple-180x180.png sizes=180x180 type=image/png>");
        respWriter.println("<link rel=stylesheet href=mainstyle.css>");
        respWriter.println("<script src=main.js></script>");
        respWriter.println("</head>");
        respWriter.println("<body>");
        respWriter.println("<button type=button id=showvolume onclick=getVolume()>&#9738</button>");
        respWriter.println("<input hidden id=volume type=range oninput=setVolume() min=0 max=5 title=volume>");
        respWriter.println("<iframe hidden id=tracks title=meta></iframe>");
        respWriter.println("<script>document.getElementById(\"tracks\").src=\"data:text/plain,ok\"</script>");
        respWriter.println("<img hidden id=trackspicture title=picture>");
        respWriter.println("<button type=button hidden id=hidetracks onclick=hideTracks()>&#9769</button>");
        respWriter.println("<dl id=albums class=scroll>");
        final String[] record = new String[2];
        record[0] = "";
        record[1] = "";
        albums.forEach((album, albumPathList) -> albumPathList.forEach((albumPath) -> {
            String albumUI = album.replace("fuckingslash", "/").replace("fuckingblackstar", "&#9733").replace("fuckingplus", "&#43").replace(" anD ", " & ");
            String onClick = "<b onclick=getTracks(\"" + (albumPath + "/" + album).replace(" ", "&") + "\")>&#9738;</b>";
            if (!record[0].equals(albumUI.split("___")[0])) {
                record[0] = albumUI.split("___")[0];
                respWriter.println("<dt>" + record[0] + "</dt>");
            }
            if (albumUI.split("___").length == 1) {
                record[1] = "";
            } else {
                record[1] = "&nbsp;&nbsp;" + albumUI.split("___")[1];
            }
            respWriter.println("<dd>" + onClick + "<small>" + record[1] + "</small></dd>");
        }));
        respWriter.println("</dl>");
        respWriter.println("</body>");
        respWriter.println("</html>");
    }
}
