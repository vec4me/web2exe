use std::fs;
use tao::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoopBuilder},
    window::WindowBuilder,
};
use wry::WebViewBuilder;

fn main() -> wry::Result<()> {
    let event_loop = EventLoopBuilder::new().build();

    let html = fs::read_to_string("index.html")
        .expect("Failed to read index.html");

    #[cfg(target_os = "windows")]
    let (window, _webview) = {
        use wry::WebViewBuilderExtWindows;

        let window = WindowBuilder::new()
            .with_decorations(true)
            .with_title("something")
            .build(&event_loop)
            .unwrap();

        let webview = WebViewBuilder::new()
            .with_html(&html)
            .with_additional_browser_args("--enable-unsafe-webgpu")
            .build(&window)?;

        (window, webview)
    };

    #[cfg(any(target_os = "macos", target_os = "ios", target_os = "android"))]
    let (window, _webview) = {
        let window = WindowBuilder::new()
            .with_decorations(false)
            .with_title("something")
            .build(&event_loop)
            .unwrap();

        let webview = WebViewBuilder::new()
            .with_html(&html)
            .build(&window)?;

        (window, webview)
    };

    #[cfg(all(unix, not(any(target_os = "macos", target_os = "ios", target_os = "android"))))]
    let (window, _webview) = {
        use tao::platform::unix::WindowExtUnix;
        use wry::WebViewBuilderExtUnix;

        let window = WindowBuilder::new()
            .with_decorations(false)
            .with_title("something")
            .build(&event_loop)
            .unwrap();

        let vbox = window.default_vbox().unwrap();
        let webview = WebViewBuilder::new()
            .with_html(&html)
            .build_gtk(vbox)?;

        (window, webview)
    };

    event_loop.run(move |event, _, control_flow| {
        *control_flow = ControlFlow::Wait;
        if let Event::WindowEvent {
            event: WindowEvent::CloseRequested,
            ..
        } = event
        {
            *control_flow = ControlFlow::Exit;
        }
    });
}
