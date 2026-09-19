#ifndef DEV_TOOL_PARAMETER_TABLE_PARAMETERNAMEDELEGATE_H
#define DEV_TOOL_PARAMETER_TABLE_PARAMETERNAMEDELEGATE_H

#include <QStyledItemDelegate>

namespace parameter_table {

/**
 * @class ParameterNameDelegate
 * @brief Draws the name column, which carries the tree structure.
 *
 * The default delegate draws one run of text per cell, which is not enough for this column:
 * - the part of a name matching the filter is highlighted, so that a match is findable in a long
 *   list without reading every row;
 * - a group heading is followed by the number of parameters underneath it, dimmed, so the count
 *   does not read as part of the name.
 *
 * Everything else about the cell — background, selection, the expander and the indentation — is
 * left to the style, so the column keeps matching the rest of the tree.
 */
class ParameterNameDelegate final : public QStyledItemDelegate {
    Q_OBJECT

public:
    /**
     * @brief Constructs a ParameterNameDelegate.
     * @param parent Optional parent object.
     */
    explicit ParameterNameDelegate(QObject* parent = nullptr);

    /**
     * @brief Sets the text to highlight in the drawn names.
     *
     * The delegate only draws; it does not filter. This is the same text the proxy model
     * filters by, handed over so that the two agree on what counts as a match.
     *
     * @param text The matched substring, or an empty string to highlight nothing.
     */
    void setHighlightedText(const QString& text);

    /**
     * @brief Paints one cell of the name column.
     * @param painter The painter to draw with.
     * @param option Style options for the cell.
     * @param index The model index of the cell being drawn.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /**
     * @brief Returns the space one cell of the name column wants.
     * @param option Style options for the cell.
     * @param index The model index of the cell being measured.
     * @return The size of the cell, with room for the group member count.
     */
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString highlighted_text_;
};

}  // namespace parameter_table

#endif  // DEV_TOOL_PARAMETER_TABLE_PARAMETERNAMEDELEGATE_H
