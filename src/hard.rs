use tao::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoopBuilder},
    window::WindowBuilder,
};
use wry::WebViewBuilder;

fn main() -> wry::Result<()> {
    let event_loop = EventLoopBuilder::new().build();
    let window = WindowBuilder::new()
        .with_decorations(false)
        .with_title("Hello")
        .build(&event_loop)
        .unwrap();

    const HTML: &str = r#"
    <!DOCTYPE html>
    <html>
      <body style="display:flex;justify-content:center;align-items:center;height:100vh;margin:0;">
        <h1>Hello</h1>
      </body>
    </html>
    "#;

    #[cfg(any(target_os = "windows", target_os = "macos", target_os = "ios", target_os = "android"))]
    let _webview = WebViewBuilder::new()
        .with_html(HTML)
        .build(&window)?;

    #[cfg(all(unix, not(any(target_os = "macos", target_os = "ios", target_os = "android"))))]
    let _webview = {
        use tao::platform::unix::WindowExtUnix;
        use wry::WebViewBuilderExtUnix;
        let vbox = window.default_vbox().unwrap();
        WebViewBuilder::new().with_html(HTML).build_gtk(vbox)?
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