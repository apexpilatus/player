package controllers;

import beans.Ipc;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;

@RestController
public class Play {
    @GetMapping("/play")
    void play(@RequestParam("album") String album, @RequestParam(name = "track", required = false, defaultValue = "01.flac") String track, HttpServletResponse resp, Ipc ipc) throws IOException {
        ipc.player0Play(album, track);
        resp.setContentType("text/plain");
        resp.getWriter().write("ok");
    }
}
