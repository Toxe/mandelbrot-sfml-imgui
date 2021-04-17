#include "app.h"
#include "gradient.h"
#include "supervisor.h"
#include "ui.h"
#include "util/mutex_timer.h"

MutexTimer mutex_timer_app_render{"app.mtx_", "App::render"};
MutexTimer mutex_timer_app_update_texture1{"app.mtx_", "App::update_texture(image)"};
MutexTimer mutex_timer_app_update_texture2{"app.mtx_", "App::update_texture(pixels)"};
MutexTimer mutex_timer_app_resize_texture{"app.mtx_", "App::resize_texture"};
MutexTimer mutex_timer_worker_combine_iterations_histogram{"mtx", "worker_combine_iterations_histogram"};
MutexTimer mutex_timer_message_queue_send{"message_queue", "MessageQueue::send"};
MutexTimer mutex_timer_message_queue_wait_for_message{"message_queue", "MessageQueue::wait_for_message"};
MutexTimer mutex_timer_message_queue_clear{"message_queue", "MessageQueue::clear"};
MutexTimer mutex_timer_message_queue_empty{"message_queue", "MessageQueue::empty"};
MutexTimer mutex_timer_message_queue_size{"message_queue", "MessageQueue::size"};

int main(int argc, char* argv[])
{
    CLI cli(argc, argv);
    App app(cli);
    UI ui(app, cli);

    const auto gradient = load_gradient("assets/gradients/benchmark.gradient");
    auto supervisor = supervisor_start(app, cli.num_threads(), gradient);

    while (app.window().isOpen()) {
        app.next_frame();
        app.poll_events(ui);
        ui.render(app);
        app.render();
    }

    ui.shutdown();
    supervisor_shutdown(supervisor);

    mutex_timer_app_render.summary();
    mutex_timer_app_update_texture1.summary();
    mutex_timer_app_update_texture2.summary();
    mutex_timer_app_resize_texture.summary();
    mutex_timer_worker_combine_iterations_histogram.summary();
    mutex_timer_message_queue_send.summary();
    mutex_timer_message_queue_wait_for_message.summary();
    mutex_timer_message_queue_clear.summary();
    mutex_timer_message_queue_empty.summary();
    mutex_timer_message_queue_size.summary();
}
