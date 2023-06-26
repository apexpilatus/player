package controllers;

import beans.Ipc;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;

@RestController
public class Battery {
    @GetMapping("/battery")
    void getBattery(HttpServletResponse resp, Ipc ipc) throws IOException {
        resp.setContentType("text/plain");
        resp.setCharacterEncoding("utf-8");
        resp.setHeader("Cache-Control", "no-cache");
        resp.setHeader("X-Content-Type-Options", "nosniff");
        resp.getWriter().write(ipc.action4GetBattery());
    }
}
