#pragma once

#include <QComboBox>
#include <QString>

#include <cad/meta/metalinewidth.h>
#include <widgets/linepatternpainter.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"

/**
 * \brief Dropdown select for line widths.
 * Line widths are hardcoded in the constructor
 */
class LineWidthSelect : public QComboBox {
    Q_OBJECT

    public:
        /**
         * \brief Create widget
         * \param parent Pointer to parent widget
         * \param showByLayer Add "ByLayer" option
         * \param showByBlock Add "ByBlock" option
         */
        LineWidthSelect(QWidget* parent = 0, bool showByLayer = false, bool showByBlock = false);

        /**
         * \brief Return selected line width
         * \return Pointer to MetaLineWidthByValue
         */
        lc::MetaLineWidthByValue_CSPtr lineWidth();

        /**
         * \brief Select a new width.
         * \param width New width
         * Does nothing if the width is not present in the values.
         */
        void setWidth(double width);

    public slots:
        /**
         * \brief Event when a new layer is selected.
         * \param layer New selected layer
         * This function update the "ByLayer" preview
         */
        void onLayerChanged(lc::Layer_CSPtr layer);

    private:
        void createEntries();

        QSize qIconSize;

        std::map<QString, double> values;
};