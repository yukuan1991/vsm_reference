#pragma once
#include "qt-tools/video_player.hpp"
#include "utility/memory.hpp"
#include <QWidget>
#include <QVBoxLayout>
#include "utility/raii.hpp"
#include <QHBoxLayout>
#include <QToolButton>
#include <QSlider>
#include <QStyle>
#include <QLabel>


class video_widget : public QWidget
{
    Q_OBJECT
signals:
    void state_changed (video_player::state_enum);
public:

    ~video_widget () override  { }
    auto set_mute (bool b) { player_.set_mute(b); }
    auto is_mute () { return player_.is_mute(); }
    auto set_file (const QString& file) { player_.set_file(file); }
    auto state () { return player_.state (); }
    static unique_ptr<video_widget> make (const QString& file = {}, QWidget* parent = nullptr)
    {
        auto ret = unique_ptr<video_widget> (new video_widget (parent));
        if (!ret->init())
        {
            return nullptr;
        }

        if (!file.isEmpty())
        {
            ret->set_file(file);
        }

        return ret;
    }

protected:
    video_widget (QWidget* parent)
        :QWidget (parent)
    {
        connect (&player_, &video_player::state_changed, this, &video_widget::state_changed);
    }

    bool init ()
    {
        setMinimumSize({200, 150});
        auto v_layout = make_unique<QVBoxLayout> (this);
        v_layout->setObjectName("v");
        v_layout->setSpacing(0);
        v_layout->setContentsMargins(1, 1, 1, 1);
        v_layout->addWidget(&player_);
        SCOPE_EXIT { setLayout(v_layout.release()); };
        SCOPE_EXIT { v_layout->setStretch(0, 1); };

        {
            auto h_layout = make_unique<QHBoxLayout> (this);
            h_layout->setObjectName("h");
            SCOPE_EXIT { v_layout->addLayout(h_layout.release()); };
            int h_index = 0;
            { /// mute
                auto mute = make_unique<QToolButton> (this);
                SCOPE_EXIT { h_index ++; };
                SCOPE_EXIT { h_layout->addWidget(mute.release()); };

                mute->setIcon(this->style()->standardIcon (is_mute () ?
                                                               QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
                connect (&player_, &video_player::mute_changed, [mute = mute.get(), this] (bool b)
                {
                    mute->setIcon (this->style()->standardIcon(b ?
                                                                   QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
                });
                connect (mute.get(), &QToolButton::clicked, [this]
                {
                    set_mute(!is_mute());
                });
            }

            { /// volume bar
                auto volume_bar = make_unique<QSlider> (this);
                SCOPE_EXIT { h_index ++; };
                SCOPE_EXIT { h_layout->addWidget (volume_bar.release()); };

                volume_bar->setMaximum(100);
                volume_bar->setMinimum(0);
                volume_bar->setEnabled(true);
                volume_bar->setPageStep(10);
                volume_bar->setSliderPosition(100);
                volume_bar->setOrientation(Qt::Horizontal);
                volume_bar->setMouseTracking(true);

                connect(volume_bar.get(), &QSlider::valueChanged, [player_ = &player_] (int value)
                {
                    player_->set_volume(static_cast<float> (value) / 100);
                });
                connect (&player_, &video_player::volume_changed, [volume_bar = volume_bar.get()] (float value)
                {
                    volume_bar->setValue(static_cast<int>(value * 100));
                });
            }

            { /// play_pause
                auto play_pause = make_unique<QToolButton> (this);
                SCOPE_EXIT { h_index ++; };
                SCOPE_EXIT { h_layout->addWidget(play_pause.release()); };

                play_pause->setIcon(this->style()->standardIcon (state() == video_player::playing_state ?
                                                                     QStyle::SP_MediaPause: QStyle::SP_MediaPlay));
                connect (&player_, &video_player::state_changed, [play_pause = play_pause.get(), this] (auto&& state)
                {
                    play_pause->setIcon(this->style()->standardIcon (state == video_player::playing_state ?
                                                                         QStyle::SP_MediaPause: QStyle::SP_MediaPlay));
                });

                connect(play_pause.get(), &QToolButton::clicked, [player_ = &player_]
                {
                    auto state = player_->state();
                    if (state == video_player::playing_state)
                    {
                        player_->pause_video(true);
                    }
                    else if (state == video_player::paused_state)
                    {
                        player_->pause_video(false);
                    }
                    else
                    {
                        player_->play_video();
                    }
                });
            }

            { /// stop
                auto stop = make_unique<QToolButton> (this);
                SCOPE_EXIT { h_index ++; };
                SCOPE_EXIT { h_layout->addWidget(stop.release()); };

                stop->setIcon(this->style()->standardIcon (QStyle::SP_MediaStop));
                stop->setEnabled(state() != video_player::stopped_state);

                connect (stop.get(), &QToolButton::clicked, [player_ = &player_]
                {
                    player_->stop_video();
                });
                connect(&player_, &video_player::state_changed, [stop = stop.get()] (auto && state)
                {
                    stop->setEnabled(state != video_player::stopped_state);
                });
            }

            { /// progress
                auto progress = make_unique<QSlider> (this);
                SCOPE_EXIT { h_index ++; };
                SCOPE_EXIT { h_layout->setStretch(h_index, 1); };
                SCOPE_EXIT { h_layout->addWidget(progress.release()); };
                progress->setOrientation(Qt::Horizontal);
                progress->setEnabled(state () == video_player::stopped_state);
                progress->setTracking(true);
                auto conn  = make_shared<QMetaObject::Connection> ();
                connect (&player_, &video_player::state_changed, [progress = progress.get()] (auto && state)
                {
                    progress->setEnabled(state != video_player::stopped_state);
                });
                connect (&player_, &video_player::duration_changed, [progress = progress.get()] (auto && duration)
                {
                    progress->setMaximum(static_cast<int> (duration));
                });
                *conn = connect (&player_, &video_player::position_changed, [progress = progress.get()] (auto && position)
                {
                    progress->setValue(static_cast<int> (position));
                });
                connect(progress.get(), &QSlider::sliderPressed, [conn]
                {
                    disconnect(*conn);
                });
                connect (progress.get(), &QSlider::sliderReleased, [conn, player_ = &player_, progress = progress.get ()]
                {
                    player_->set_position(progress->value());
                    *conn = QObject::connect (player_, &video_player::position_changed, [progress] (auto && position)
                    {
                        progress->setValue(static_cast<int> (position));
                    });
                });
            }

            { /// time_label
                auto time_label = make_unique<QLabel> (this);
                SCOPE_EXIT { h_index ++; };
                SCOPE_EXIT { h_layout->addWidget(time_label.release()); };

                auto sp_duration = make_shared<QString> ();
                auto sp_position = make_shared<QString> ();

                auto get_time_str = [] (qint64 time_value)
                {
                    int hour = static_cast<int> (time_value / 3600 / 1000);
                    int minutes = time_value / 1000 % 3600 / 60;
                    int seconds = time_value / 1000 % 3600 % 60;
                    int one_tenth = time_value % 1000 / 100;

                    return QString::asprintf("%d:%02d:%02d.%1d", hour, minutes, seconds, one_tenth);
                };

                connect(&player_, &video_player::position_changed,
                        [sp_duration, sp_position, get_time_str, time_label = time_label.get()] (auto && position)
                {
                    *sp_position = get_time_str (position);
                    time_label->setText(*sp_position + "/" + *sp_duration);
                });

                connect(&player_, &video_player::duration_changed,
                        [player_ = &player_, sp_duration, sp_position, get_time_str, time_label = time_label.get()]
                        (auto && duration)
                {
                    if (player_->state() == video_player::stopped_state)
                    {
                        *sp_position = "00:00:00.0";
                        if (duration > 0)
                        {
                            *sp_duration = get_time_str (duration);
                        }
                    }
                    else
                    {
                        *sp_duration = get_time_str (duration);
                    }
                    time_label->setText(*sp_position + "/" + *sp_duration);
                });
            }
        }
        return true;
    }

    void hideEvent (QHideEvent* event) override
    {
        player_.stop_video();
        QWidget::hideEvent(event);
    }
private:
    video_player player_ { this };
};
