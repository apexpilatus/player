package controllers;

import beans.PlayerIpc;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;

@RestController
public class Power {
    @GetMapping("/power")
    void play(HttpServletResponse resp) throws IOException {
        resp.setContentType("text/plain");
        resp.setCharacterEncoding("utf-8");
        resp.setHeader("Cache-Control", "no-cache, no-store");
        resp.setHeader("X-Content-Type-Options", "nosniff");
        resp.getWriter().write("12");
    }
}
