package controllers;

import beans.MetaIpc;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RestController
public class MainPage {
    @GetMapping("/")
    void mainPage(HttpServletResponse resp, MetaIpc metaIpc) throws IOException {
        Map<String, List<String>> albums = metaIpc.meta0GetAlbums();
        resp.setContentType("text/html");
        resp.setCharacterEncoding("utf-8");
        resp.setHeader("Cache-Control", "no-cache, no-store");
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
        if (System.getProperty("os.name").contains("Linux") && new File("/sys/class/power_supply/BAT0/capacity").exists()){
		respWriter.println("<strong id=power>&#9889;</strong>");
        	respWriter.println("<script>getpower()</script>");
        	respWriter.println("<script>setInterval(getpower, 60000)</script>");
	}
        respWriter.println("<button type=button id=showvolume onclick=getVolume()>&#9738;</button>");
        respWriter.println("<input hidden id=volume type=range oninput=setVolume() min=0 max=5 title=volume>");
        respWriter.println("<iframe hidden id=tracks title=meta></iframe>");
        respWriter.println("<script>document.getElementById(\"tracks\").src=\"data:text/plain,ok\"</script>");
        respWriter.println("<img hidden id=trackspicture title=picture>");
        respWriter.println("<button type=button hidden id=hidetracks onclick=hideTracks()>&#9769</button>");
        respWriter.println("<dl id=albums class=scroll>");
        Map<String, String> record = new HashMap<>();
        record.put("artist", "");
        record.put("album", "");
        albums.forEach((album, albumPathList) -> albumPathList.forEach((albumPath) -> {
            String albumUI = album.replace("SlasHHH", "/").replace("StaRRR", "&#9733;").replace("PluSSS", "&#43;").replace("AnDDD", "&amp;").replace("QuoteSSS", "&quot;");
            String onClick = "<b onclick=getTracks(\"" + (albumPath + "/" + album).replace(" ", "&") + "\")>&#9738;</b>";
            if (!record.get("artist").equals(albumUI.split("___")[0])) {
                record.put("artist", albumUI.split("___")[0]);
                respWriter.println("<dt>" + record.get("artist") + "</dt>");
            }
            if (albumUI.split("___").length == 1) {
                record.put("album", "");
            } else {
                record.put("album", "&nbsp;&nbsp;" + albumUI.split("___")[1]);
            }
            respWriter.println("<dd>" + onClick + "<small>" + record.get("album") + "</small></dd>");
        }));
        respWriter.println("</dl>");
        respWriter.println("</body>");
        respWriter.println("</html>");
    }
}
