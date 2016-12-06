#ifndef SCREEN_SAVER_H
#define SCREEN_SAVER_H

#include <QDialog>
#include <QApplication>
#include <QPixmap>
#include <experimental/optional>

enum class copy_to_clip : bool
{
    no = false, yes = true
};
class screen_saver : public QDialog
{
    Q_OBJECT

public:

    static QPixmap grab (copy_to_clip opt = copy_to_clip::yes);
    static QPixmap snapshot (WId win_id = 0);

protected:
    screen_saver(QPixmap& pix_ref, QWidget *parent = nullptr);
    ~screen_saver();
    void showEvent (QShowEvent* event) override;
    void paintEvent (QPaintEvent* event) override;
    void mousePressEvent (QMouseEvent* event) override;
    void mouseReleaseEvent (QMouseEvent* event) override;
    void mouseMoveEvent (QMouseEvent* event) override;
    void after_grab (QMouseEvent* event);
private:
    QPixmap full_screen_ = snapshot ();
    QPixmap& screen_shot_;
    std::experimental::optional<QPoint> pressed_pos_;
private: // settings
    static constexpr auto font_size = 15;
};

#endif // SCREEN_SAVER_H
