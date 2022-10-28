#include "filter.h"

#include "animation.h"

namespace kaputt
{
void FilterPipeline::filter(std::vector<std::string_view>& anims, const RE::Actor* attacker, const RE::Actor* victim) const
{
    auto anim_manager = AnimManager::getSingleton();
    auto tag_result   = Tagger::tag(tagger_list, attacker, victim);

    std::erase_if(anims, [&](std::string_view edid) {
        StrSet exp_tags    = {};
        auto   p_orig_tags = anim_manager->getTags(edid);
        if (!p_orig_tags)
            return true;

        for (const auto& [from, to] : tagexp_list)
            if (p_orig_tags->contains(from))
                exp_tags.merge(const_cast<StrSet&>(to));
        exp_tags.merge(*const_cast<StrSet*>(p_orig_tags));

        return !(std::ranges::all_of(tag_result.required_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }) &&
                 std::ranges::none_of(tag_result.banned_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }));
    });
}
} // namespace kaputt
