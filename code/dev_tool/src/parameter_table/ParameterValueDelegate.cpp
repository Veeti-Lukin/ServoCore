#include "parameter_table/ParameterValueDelegate.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <limits>

#include "parameter_system/parameter_type_mappings.h"
#include "parameter_table/ParameterTableWidget.h"

namespace parameter_table {

ParameterValueDelegate::ParameterValueDelegate(QVector<RowData>& rows, QObject* parent)
    : QStyledItemDelegate(parent), rows_(rows) {}

QWidget* ParameterValueDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const {
    // The caller will claim the ownership of the dynamically allocated widget for "parent"

    // TODO When parameter limits are done get them from the parameter system as well, for now the limits are hard coded
    // editor->setMinimum(0);
    // editor->setMaximum(100);

    const RowData& row = rows_[index.row()];

    switch (row.meta_data.type) {
        case parameter_system::ParameterType::uint8:
        case parameter_system::ParameterType::uint16:
        case parameter_system::ParameterType::uint32:
        case parameter_system::ParameterType::uint64:
        case parameter_system::ParameterType::int8:
        case parameter_system::ParameterType::int16:
        case parameter_system::ParameterType::int32:
        case parameter_system::ParameterType::int64: {
            QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
            editor->setDecimals(0);    // Prevents floating-point numbers
            editor->setSingleStep(1);  // Ensure only whole numbers increment
            // Enforce integer-only input by blocking non-numeric characters
            editor->setKeyboardTracking(false);
            editor->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
            editor->setFrame(false);
            editor->setMinimum(std::numeric_limits<int64_t>::min());
            editor->setMaximum(std::numeric_limits<uint64_t>::max());
            return editor;
        }
        case parameter_system::ParameterType::floating_point:
        case parameter_system::ParameterType::double_float: {
            QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
            editor->setMinimum(std::numeric_limits<parameter_system::ParameterTypeMapping<
                                   parameter_system::ParameterType::double_float>::type>::min());
            editor->setMaximum(std::numeric_limits<parameter_system::ParameterTypeMapping<
                                   parameter_system::ParameterType::double_float>::type>::max());
            editor->setFrame(false);
            return editor;
        }

        case parameter_system::ParameterType::boolean: {
            QComboBox* editor = new QComboBox(parent);
            editor->setFrame(false);
            editor->addItem("false", false);
            editor->addItem("true", true);
            return editor;
        }

        case parameter_system::ParameterType::none:
            break;
    }
}

void ParameterValueDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    const RowData& row = rows_[index.row()];

    switch (row.meta_data.type) {
        case parameter_system::ParameterType::uint8:
        case parameter_system::ParameterType::uint16:
        case parameter_system::ParameterType::uint32:
        case parameter_system::ParameterType::uint64: {
            uint64_t        value    = rows_[index.row()].value.value<uint64_t>();
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            spin_box->setValue(value);
            break;
        }
        case parameter_system::ParameterType::int8:
        case parameter_system::ParameterType::int16:
        case parameter_system::ParameterType::int32:
        case parameter_system::ParameterType::int64: {
            int64_t         value    = rows_[index.row()].value.value<int64_t>();
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            spin_box->setValue(value);
            break;
        }
        case parameter_system::ParameterType::floating_point:
        case parameter_system::ParameterType::double_float: {
            double          value    = rows_[index.row()].value.value<double>();
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            spin_box->setValue(value);
            break;
        }
        case parameter_system::ParameterType::boolean: {
            bool       value     = rows_[index.row()].value.value<bool>();
            QComboBox* combo_box = static_cast<QComboBox*>(editor);
            combo_box->setCurrentIndex(value);  // 0 false 1 true
            break;
        }
        case parameter_system::ParameterType::none:
            break;
    }
}

void ParameterValueDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    const RowData& row = rows_[index.row()];

    switch (row.meta_data.type) {
        case parameter_system::ParameterType::uint8: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            uint8_t         value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::uint16: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            uint16_t        value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::uint32: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            uint32_t        value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::uint64: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            uint64_t        value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::int8: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            int8_t          value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::int16: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            int16_t         value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::int32: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            int32_t         value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::int64: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            int64_t         value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::floating_point: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            float           value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::double_float: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            double          value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::boolean: {
            QComboBox* combo_box = static_cast<QComboBox*>(editor);
            bool       value     = combo_box->itemData(combo_box->currentIndex()).toBool();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterType::none:
            break;
    }
}

void ParameterValueDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                                  const QModelIndex& index) const {
    editor->setGeometry(option.rect);
}

}  // namespace parameter_table