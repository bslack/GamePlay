#include "Base.h"
#include "Technique.h"
#include "Material.h"
#include "Node.h"

namespace gameplay
{

Technique::Technique(const char* id, Material* material)
    : _id(id ? id : ""), _material(material)
{
    assert(material);

    RenderState::_parent = material;
}

Technique::~Technique()
{
    // Destroy all the passes.
    for (unsigned int i = 0, count = _passes.size(); i < count; ++i)
    {
        SAFE_RELEASE(_passes[i]);
    }
}

const char* Technique::getId() const
{
    return _id.c_str();
}

unsigned int Technique::getPassCount() const
{
    return _passes.size();
}

Pass* Technique::getPass(unsigned int index) const
{
    assert(index < _passes.size());

    return _passes[index];
}

Pass* Technique::getPass(const char* id) const
{
    for (unsigned int i = 0, count = _passes.size(); i < count; ++i)
    {
        Pass* pass = _passes[i];
        if (strcmp(pass->getId(), id) == 0)
        {
            return pass;
        }
    }

    return NULL;
}

Technique* Technique::clone(Material* material, NodeCloneContext &context) const
{
    Technique* technique = new Technique(getId(), material);
    technique->_material = material;
    for (std::vector<Pass*>::const_iterator it = _passes.begin(); it != _passes.end(); ++it)
    {
        Pass* pass = *it;
        Pass* passCopy = pass->clone(technique, context);
        technique->_passes.push_back(passCopy);
    }
    RenderState::cloneInto(technique, context);
    return technique;
}

}
