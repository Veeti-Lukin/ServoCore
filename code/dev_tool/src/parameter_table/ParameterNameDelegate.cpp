#include "parameter_table/ParameterNameDelegate.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QStyle>

#include "parameter_table/common.h"

namespace parameter_table {
namespace {

/// Space between a group heading and the member count drawn after it, in pixels.
constexpr int K_COUNT_GAP = 8;

/// Fill behind a filter match, and the text drawn on top of it. Deliberately a fixed pair rather
/// than palette colours: the palette's highlight is the *selection* colour, which on this
/// project's dark theme comes out near black and turns a match into a hole in the row instead of
/// a mark on it. Amber with black text is the colour a search hit is expected to be, and it
/// carries the same meaning over a light background, a dark one, and a selected row.
const QColor K_MATCH_BACKGROUND = QColor(0xE8, 0xA3, 0x3D);
const QColor K_MATCH_TEXT       = QColor(Qt::black);

/// @brief Counts the parameter rows under an index, as the view currently shows them.
int countParameterRows(const QAbstractItemModel* model, const QModelIndex& parent) {
    int count = 0;

    const int row_count = model->rowCount(parent);
    for (int row = 0; row < row_count; row++) {
        const QModelIndex child = model->index(row, 0, parent);
        if (child.data(k_row_kind_role).toInt() == static_cast<int>(RowData::Kind::parameter)) count++;
        count += countParameterRows(model, child);
    }

    return count;
}

/// @brief The "(12)" drawn after a group heading, or an empty string on a parameter row.
QString memberCountText(const QModelIndex& index) {
    if (index.data(k_row_kind_role).toInt() != static_cast<int>(RowData::Kind::group)) return {};

    const QAbstractItemModel* model = index.model();
    if (model == nullptr) return {};

    // Counted through the model the view is showing, which is the filtered one. A group that
    // says "(3)" while three of its twelve parameters are listed underneath is answering the
    // question the reader is actually asking.
    return QStringLiteral("(%1)").arg(countParameterRows(model, index));
}

}  // namespace

ParameterNameDelegate::ParameterNameDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void ParameterNameDelegate::setHighlightedText(const QString& text) { highlighted_text_ = text; }

void ParameterNameDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
    QStyleOptionViewItem styled_option = option;
    initStyleOption(&styled_option, index);

    // Take the text out of the style option and let the style draw everything else: the
    // background, the selection, the focus rectangle and the branch indicator all keep matching
    // the rest of the tree that way. Only the text itself is drawn by hand below.
    const QString name = styled_option.text;
    styled_option.text.clear();

    const QWidget* widget = styled_option.widget;
    QStyle*        style  = (widget != nullptr) ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &styled_option, painter, widget);

    const QRect text_rect = style->subElementRect(QStyle::SE_ItemViewItemText, &styled_option, widget);
    if (name.isEmpty() || text_rect.isEmpty()) return;

    const QFontMetrics metrics(styled_option.font);
    const QString      count_text  = memberCountText(index);
    const int          count_width = count_text.isEmpty() ? 0 : metrics.horizontalAdvance(count_text) + K_COUNT_GAP;

    // The member count is never elided away; the name gives up the room for it instead.
    const QString elided_name = metrics.elidedText(name, Qt::ElideRight, text_rect.width() - count_width);

    painter->save();
    painter->setFont(styled_option.font);

    const QColor text_color = ((styled_option.state & QStyle::State_Selected) != 0)
                                  ? styled_option.palette.color(QPalette::HighlightedText)
                                  : styled_option.palette.color(QPalette::Text);

    int pen_x = text_rect.left();

    auto drawSegment = [&](const QString& segment, bool highlighted) {
        if (segment.isEmpty()) return;

        const QRect segment_rect(pen_x, text_rect.top(), metrics.horizontalAdvance(segment), text_rect.height());
        if (highlighted) painter->fillRect(segment_rect, K_MATCH_BACKGROUND);

        painter->setPen(highlighted ? K_MATCH_TEXT : text_color);
        painter->drawText(segment_rect, Qt::AlignLeft | Qt::AlignVCenter, segment);
        pen_x += segment_rect.width();
    };

    // Matched against the elided text rather than the full name, so that the highlight lands on
    // the characters actually on screen.
    const int match_start =
        highlighted_text_.isEmpty() ? -1 : elided_name.indexOf(highlighted_text_, 0, Qt::CaseInsensitive);

    if (match_start < 0) {
        drawSegment(elided_name, false);
    } else {
        const int match_length = highlighted_text_.length();
        drawSegment(elided_name.left(match_start), false);
        drawSegment(elided_name.mid(match_start, match_length), true);
        drawSegment(elided_name.mid(match_start + match_length), false);
    }

    if (!count_text.isEmpty()) {
        const QRect count_rect(pen_x + K_COUNT_GAP, text_rect.top(), metrics.horizontalAdvance(count_text),
                               text_rect.height());

        painter->setPen(styled_option.palette.color(QPalette::Disabled, QPalette::Text));
        painter->drawText(count_rect, Qt::AlignLeft | Qt::AlignVCenter, count_text);
    }

    painter->restore();
}

QSize ParameterNameDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);

    const QString count_text = memberCountText(index);
    if (count_text.isEmpty()) return size;

    QStyleOptionViewItem styled_option = option;
    initStyleOption(&styled_option, index);

    const QFontMetrics metrics(styled_option.font);
    size.setWidth(size.width() + K_COUNT_GAP + metrics.horizontalAdvance(count_text));
    return size;
}

}  // namespace parameter_table
