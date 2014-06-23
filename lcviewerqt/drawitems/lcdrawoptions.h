#ifndef LCDRAWOPTIONS_H
#define LCDRAWOPTIONS_H

#include <QColor>

class LcDrawOptions {
    public:
        LcDrawOptions();

        const QColor& selectedColor() const;

    private:
        QColor  _selectedColor;
};

#endif // LCDRAWOPTIONS_H
