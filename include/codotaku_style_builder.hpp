#include <yoga/Yoga.h>
#include <cstdint>
#include <stddef.h>

class StyleBuilder {
    YGNodeRef node_;

public:
    explicit StyleBuilder(YGNodeRef node) : node_(node) {
    }

    // Direction & Layout
    StyleBuilder &direction(YGDirection direction) {
        YGNodeStyleSetDirection(node_, direction);
        return *this;
    }

    StyleBuilder &flexDirection(YGFlexDirection flexDirection) {
        YGNodeStyleSetFlexDirection(node_, flexDirection);
        return *this;
    }

    StyleBuilder &flexWrap(YGWrap flexWrap) {
        YGNodeStyleSetFlexWrap(node_, flexWrap);
        return *this;
    }

    StyleBuilder &display(YGDisplay display) {
        YGNodeStyleSetDisplay(node_, display);
        return *this;
    }

    StyleBuilder &overflow(YGOverflow overflow) {
        YGNodeStyleSetOverflow(node_, overflow);
        return *this;
    }

    StyleBuilder &boxSizing(YGBoxSizing boxSizing) {
        YGNodeStyleSetBoxSizing(node_, boxSizing);
        return *this;
    }

    // Alignment & Justification
    StyleBuilder &justifyContent(YGJustify justifyContent) {
        YGNodeStyleSetJustifyContent(node_, justifyContent);
        return *this;
    }

    StyleBuilder &justifyItems(YGJustify justifyItems) {
        YGNodeStyleSetJustifyItems(node_, justifyItems);
        return *this;
    }

    StyleBuilder &justifySelf(YGJustify justifySelf) {
        YGNodeStyleSetJustifySelf(node_, justifySelf);
        return *this;
    }

    StyleBuilder &alignContent(YGAlign alignContent) {
        YGNodeStyleSetAlignContent(node_, alignContent);
        return *this;
    }

    StyleBuilder &alignItems(YGAlign alignItems) {
        YGNodeStyleSetAlignItems(node_, alignItems);
        return *this;
    }

    StyleBuilder &alignSelf(YGAlign alignSelf) {
        YGNodeStyleSetAlignSelf(node_, alignSelf);
        return *this;
    }

    // Flex Properties
    StyleBuilder &flex(float flex) {
        YGNodeStyleSetFlex(node_, flex);
        return *this;
    }

    StyleBuilder &flexGrow(float flexGrow) {
        YGNodeStyleSetFlexGrow(node_, flexGrow);
        return *this;
    }

    StyleBuilder &flexShrink(float flexShrink) {
        YGNodeStyleSetFlexShrink(node_, flexShrink);
        return *this;
    }

    StyleBuilder &flexBasis(float points) {
        YGNodeStyleSetFlexBasis(node_, points);
        return *this;
    }

    StyleBuilder &flexBasisPercent(float percent) {
        YGNodeStyleSetFlexBasisPercent(node_, percent);
        return *this;
    }

    StyleBuilder &flexBasisAuto() {
        YGNodeStyleSetFlexBasisAuto(node_);
        return *this;
    }

    // Sizing
    StyleBuilder &width(float points) {
        YGNodeStyleSetWidth(node_, points);
        return *this;
    }

    StyleBuilder &widthPercent(float percent) {
        YGNodeStyleSetWidthPercent(node_, percent);
        return *this;
    }

    StyleBuilder &widthAuto() {
        YGNodeStyleSetWidthAuto(node_);
        return *this;
    }

    StyleBuilder &height(float points) {
        YGNodeStyleSetHeight(node_, points);
        return *this;
    }

    StyleBuilder &heightPercent(float percent) {
        YGNodeStyleSetHeightPercent(node_, percent);
        return *this;
    }

    StyleBuilder &heightAuto() {
        YGNodeStyleSetHeightAuto(node_);
        return *this;
    }

    StyleBuilder &aspectRatio(float aspectRatio) {
        YGNodeStyleSetAspectRatio(node_, aspectRatio);
        return *this;
    }

    // Box Model (Margin, Padding, Border, Gap)
    StyleBuilder &margin(YGEdge edge, float points) {
        YGNodeStyleSetMargin(node_, edge, points);
        return *this;
    }

    StyleBuilder &marginPercent(YGEdge edge, float percent) {
        YGNodeStyleSetMarginPercent(node_, edge, percent);
        return *this;
    }

    StyleBuilder &marginAuto(YGEdge edge) {
        YGNodeStyleSetMarginAuto(node_, edge);
        return *this;
    }

    StyleBuilder &padding(YGEdge edge, float points) {
        YGNodeStyleSetPadding(node_, edge, points);
        return *this;
    }

    StyleBuilder &paddingPercent(YGEdge edge, float percent) {
        YGNodeStyleSetPaddingPercent(node_, edge, percent);
        return *this;
    }

    StyleBuilder &border(YGEdge edge, float border) {
        YGNodeStyleSetBorder(node_, edge, border);
        return *this;
    }

    StyleBuilder &gap(YGGutter gutter, float gapLength) {
        YGNodeStyleSetGap(node_, gutter, gapLength);
        return *this;
    }

    StyleBuilder &gapPercent(YGGutter gutter, float percent) {
        YGNodeStyleSetGapPercent(node_, gutter, percent);
        return *this;
    }

    // Positioning
    StyleBuilder &positionType(YGPositionType positionType) {
        YGNodeStyleSetPositionType(node_, positionType);
        return *this;
    }

    StyleBuilder &position(YGEdge edge, float points) {
        YGNodeStyleSetPosition(node_, edge, points);
        return *this;
    }

    StyleBuilder &positionPercent(YGEdge edge, float percent) {
        YGNodeStyleSetPositionPercent(node_, edge, percent);
        return *this;
    }

    // Grid Item Placement
    StyleBuilder &gridColumnStart(int32_t start) {
        YGNodeStyleSetGridColumnStart(node_, start);
        return *this;
    }

    StyleBuilder &gridColumnEnd(int32_t end) {
        YGNodeStyleSetGridColumnEnd(node_, end);
        return *this;
    }

    StyleBuilder &gridRowStart(int32_t start) {
        YGNodeStyleSetGridRowStart(node_, start);
        return *this;
    }

    StyleBuilder &gridRowEnd(int32_t end) {
        YGNodeStyleSetGridRowEnd(node_, end);
        return *this;
    }

    // Terminal method to return the configured node
    YGNodeRef build() const {
        return node_;
    }
};
