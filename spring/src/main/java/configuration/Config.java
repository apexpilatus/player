package configuration;

import java.io.File;
import java.time.Duration;

import org.springframework.context.annotation.Configuration;
import org.springframework.http.CacheControl;
import org.springframework.web.servlet.config.annotation.EnableWebMvc;
import org.springframework.web.servlet.config.annotation.ResourceHandlerRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

@Configuration
@EnableWebMvc
public class Config implements WebMvcConfigurer {
    @Override
    public void addResourceHandlers(ResourceHandlerRegistry registry) {
        File ffkk = new File(".");
        System.out.println(ffkk.list()[0]);
        System.out.println(System.getProperties());
        registry.addResourceHandler("/*")
                .addResourceLocations("ServletContext:/")
                .setCacheControl(CacheControl.maxAge(Duration.ofDays(365)));
    }
}