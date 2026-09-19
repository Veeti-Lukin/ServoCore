#include "parameter_table/ParameterValueDelegate.h"

#include <limits>

#include <QComboBox>
#include <QDoubleSpinBox>

#include "parameter_system/parameter_type_mappings.h"

namespace parameter_table {

ParameterValueDelegate::ParameterValueDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

parameter_system::ParameterValueType ParameterValueDelegate::valueTypeOf(const QModelIndex& index) {
    const QVariant value_type = index.data(k_value_type_role);
    // Group rows carry no value type. `none` makes them fall through to the "not editable" path.
    if (!value_type.isValid()) return parameter_system::ParameterValueType::none;

    return static_cast<parameter_system::ParameterValueType>(value_type.toInt());
}

QWidget* ParameterValueDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const {
    Q_UNUSED(option)
    // The caller will claim the ownership of the dynamically allocated widget for "parent"

    // TODO When parameter limits are done get them from the parameter system as well, for now the limits are hard coded
    // editor->setMinimum(0);
    // editor->setMaximum(100);

    switch (valueTypeOf(index)) {
        case parameter_system::ParameterValueType::uint8:
        case parameter_system::ParameterValueType::uint16:
        case parameter_system::ParameterValueType::uint32:
        case parameter_system::ParameterValueType::uint64:
        case parameter_system::ParameterValueType::int8:
        case parameter_system::ParameterValueType::int16:
        case parameter_system::ParameterValueType::int32:
        case parameter_system::ParameterValueType::int64: {
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
        case parameter_system::ParameterValueType::floating_point:
        case parameter_system::ParameterValueType::double_float: {
            QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
            editor->setMinimum(std::numeric_limits<parameter_system::MapParameterValueTypeToCppType<
                                   parameter_system::ParameterValueType::double_float>::type>::min());
            editor->setMaximum(std::numeric_limits<parameter_system::MapParameterValueTypeToCppType<
                                   parameter_system::ParameterValueType::double_float>::type>::max());
            editor->setFrame(false);
            return editor;
        }

        case parameter_system::ParameterValueType::boolean: {
            QComboBox* editor = new QComboBox(parent);
            editor->setFrame(false);
            editor->addItem("false", false);
            editor->addItem("true", true);
            return editor;
        }

        case parameter_system::ParameterValueType::none:
            break;
    }

    // Returning a nullptr is how the delegate tells the view that the cell is not editable
    return nullptr;
}

void ParameterValueDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    const QVariant value = index.data(Qt::EditRole);

    switch (valueTypeOf(index)) {
        case parameter_system::ParameterValueType::uint8:
        case parameter_system::ParameterValueType::uint16:
        case parameter_system::ParameterValueType::uint32:
        case parameter_system::ParameterValueType::uint64: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            spin_box->setValue(static_cast<double>(value.value<uint64_t>()));
            break;
        }
        case parameter_system::ParameterValueType::int8:
        case parameter_system::ParameterValueType::int16:
        case parameter_system::ParameterValueType::int32:
        case parameter_system::ParameterValueType::int64: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            spin_box->setValue(static_cast<double>(value.value<int64_t>()));
            break;
        }
        case parameter_system::ParameterValueType::floating_point:
        case parameter_system::ParameterValueType::double_float: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            spin_box->setValue(value.value<double>());
            break;
        }
        case parameter_system::ParameterValueType::boolean: {
            QComboBox* combo_box = static_cast<QComboBox*>(editor);
            combo_box->setCurrentIndex(value.value<bool>());  // 0 false 1 true
            break;
        }
        case parameter_system::ParameterValueType::none:
            break;
    }
}

void ParameterValueDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    switch (valueTypeOf(index)) {
        case parameter_system::ParameterValueType::uint8: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            uint8_t         value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::uint16: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            uint16_t        value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::uint32: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            uint32_t        value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::uint64: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            uint64_t        value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::int8: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            int8_t          value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::int16: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            int16_t         value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::int32: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            int32_t         value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::int64: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            int64_t         value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::floating_point: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            float           value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::double_float: {
            QDoubleSpinBox* spin_box = static_cast<QDoubleSpinBox*>(editor);
            double          value    = spin_box->value();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::boolean: {
            QComboBox* combo_box = static_cast<QComboBox*>(editor);
            bool       value     = combo_box->itemData(combo_box->currentIndex()).toBool();
            model->setData(index, value, Qt::EditRole);
            break;
        }
        case parameter_system::ParameterValueType::none:
            break;
    }
}

void ParameterValueDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                                  const QModelIndex& index) const {
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

}  // namespace parameter_table
