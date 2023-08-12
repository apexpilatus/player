package controllers;

import beans.PlayerIpc;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;

@RestController
public class Volume {
    @PostMapping("/volume")
    void setVolume(@RequestParam("level") int targLevel, HttpServletResponse resp, PlayerIpc playerIpc) throws IOException {
        playerIpc.player1SetVol(targLevel);
        resp.setContentType("text/plain");
        resp.getWriter().write("ok");
    }

    @GetMapping("/volume")
    void getVolume(HttpServletResponse resp, PlayerIpc playerIpc) throws IOException {
        resp.setContentType("text/plain");
        resp.setCharacterEncoding("utf-8");
        resp.setHeader("Cache-Control", "no-cache");
        resp.setHeader("X-Content-Type-Options", "nosniff");
        resp.getWriter().write(playerIpc.player2GetVol());
    }
}
