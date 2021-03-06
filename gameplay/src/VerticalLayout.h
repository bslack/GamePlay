#ifndef VERTICALLAYOUT_H_
#define VERTICALLAYOUT_H_

#include "Layout.h"
#include "Container.h"

namespace gameplay
{

/**
 * Vertical layout: Controls are placed next to one another vertically until
 * the bottom-most edge of the container is reached.
 */
class VerticalLayout : public Layout
{
    friend class Form;
    friend class Container;

public:
    /**
     * Set whether this layout will start laying out controls from the bottom of the container.
     * This setting defaults to 'false', meaning controls will start at the top.
     *
     * @param bottomToTop Whether to start laying out controls from the bottom of the container.
     */
    void setBottomToTop(bool bottomToTop);

    /**
     * Get whether this layout will start laying out controls from the bottom of the container.
     *
     * @return Whether to start laying out controls from the bottom of the container.
     */
    bool getBottomToTop();

    /**
     * Get the type of this Layout.
     *
     * @return Layout::LAYOUT_VERTICAL
     */
    Layout::Type getType();

protected:
    VerticalLayout();
    virtual ~VerticalLayout();

    static VerticalLayout* create();

    void update(const Container* container);

    bool _bottomToTop;

private:
    VerticalLayout(const VerticalLayout& copy);
};

}

#endif