/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef IVW_LISTPROPERTY_H
#define IVW_LISTPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/**
 * \class ListProperty
 * \brief A property that has specified sub-properties that can be added
 * Represents a List of properties that allows to add and delete items of this list
 */
template <typename T>
class ListProperty : public CompositeProperty {
    static_assert(std::is_base_of<Property, T>::value, "T must be a property.");

private:
    std::string elementName_;
    std::unique_ptr<T> prefab_;

public:
    size_t numElements_;
    size_t maxNumElements_;

    ListProperty(std::string identifier, std::string displayName, std::string elementName = "Element",
                 std::unique_ptr<T> prefab = nullptr, size_t maxNumberOfElements = 0,
                 InvalidationLevel = InvalidationLevel::InvalidResources,
                 PropertySemantics semantics = PropertySemantics::Default);
    ListProperty(const ListProperty& rhs);
    ListProperty& operator=(const ListProperty& that);
    virtual ListProperty* clone() const override;
    virtual ~ListProperty() = default;

    void addElement();
    void deleteElement();

    OptionPropertyString elementSelection_;
    ButtonProperty addElementButton_;
    ButtonProperty deleteElementButton_;
    CompositeProperty elements_;
};

template <typename T>
ListProperty<T>::ListProperty(std::string identifier, std::string displayName,
                              std::string elementName, std::unique_ptr<T> prefab,
                              size_t maxNumberOfElements, InvalidationLevel invalidationLevel,
                              PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , elementName_(elementName)
    , prefab_(std::move(prefab))
    , addElementButton_("addElement", "Add Element")
    , deleteElementButton_("deleteElement", "Delete Element")
    , elementSelection_("elementSelection", "Element Selection")
    , maxNumElements_(maxNumberOfElements)
    , numElements_(0)
    , elements_("lightsContainer", "Lights") {
    addElementButton_.onChange(this, &ListProperty<T>::addElement);
    deleteElementButton_.onChange(this, &ListProperty<T>::deleteElement);

    addProperty(elementSelection_);
    addProperty(deleteElementButton_);
    addProperty(addElementButton_);
    addProperty(elements_);
}

template <typename T>
ListProperty<T>::ListProperty(const ListProperty<T>& rhs)
    : CompositeProperty(rhs)
    , elementName_(rhs.elementName_)
    , prefab_(rhs.prefab_->clone())
    , addElementButton_(rhs.addElementButton_)
    , deleteElementButton_(rhs.deleteElementButton_)
    , elementSelection_(rhs.elementSelection_)
    , maxNumElements_(rhs.maxNumElements_)
    , numElements_(rhs.numElements_)
    , elements_(rhs.elements_) {
    addElementButton_.onChange(this, &ListProperty<T>::addElement);
    deleteElementButton_.onChange(this, &ListProperty<T>::deleteElement);

    addProperty(elementSelection_);
    addProperty(deleteElementButton_);
    addProperty(addElementButton_);
    addProperty(elements_);
}

template <typename T>
ListProperty<T>& ListProperty<T>::operator=(const ListProperty<T>& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        elementName_ = that.elementName_;
        prefab_ = std::make_unique<T>(*(that.prefab_->clone()));
        addElementButton_ = that.addElementButton_;
        deleteElementButton_ = that.deleteElementButton_;
        elementSelection_ = that.elementSelection_;
        maxNumElements_ = that.maxNumElements_;
        numElements_ = that.numElements_;
        elements_ = that.elements_;
    }
    return *this;
}

template <typename T>
ListProperty<T>* ListProperty<T>::clone() const {
    return new ListProperty<T>(*this);
}

template <typename T>
void ListProperty<T>::addElement() {
    if (numElements_ < maxNumElements_ || maxNumElements_ == 0) {
        numElements_++;
        std::string num = std::to_string(numElements_);

        elementSelection_.addOption("elementOption_" + num, elementName_ + " " + num);

        T* property = prefab_->clone();
        property->setSerializationMode(PropertySerializationMode::All);
        property->setIdentifier("element_" + num);
        property->setDisplayName(elementName_ + " " + num);
        elements_.addProperty(property, true);
    }
}

template <typename T>
void ListProperty<T>::deleteElement() {
    if (numElements_ <= 0) return;
    std::vector<Property*> beforeDeletion = elements_.getProperties();
    int selectedElement = elementSelection_.getSelectedIndex();

    std::string identifier = (static_cast<T*>(beforeDeletion.at(selectedElement)))->getIdentifier();
    elements_.removeProperty(identifier);
    elementSelection_.removeOption(selectedElement);
    numElements_--;

    std::vector<Property*> afterDeletion = elements_.getProperties();

    size_t loopCount = 1;
    for (Property* prop : afterDeletion) {
        T* casted = static_cast<T*>(prop);
        casted->setDisplayName(elementName_ + " " + std::to_string(loopCount));
        casted->setIdentifier("element_" + std::to_string(loopCount));
        loopCount++;
    }

    elementSelection_.clearOptions();

    for (size_t i = 1; i < afterDeletion.size() + 1; i++) {
        std::string str_i = std::to_string(i);
        elementSelection_.addOption("elementOption_" + str_i, elementName_ + " " + str_i);
    }
}

}  // namespace inviwo

#endif  // IVW_LISTPROPERTY_H