#include "filter.h"

#include "kaputt.h"
#include "utils.h"

namespace kaputt
{
void FilterPipeline::filter(std::vector<std::string_view>& anims, const RE::Actor* attacker, const RE::Actor* victim, const TaggerOutput& extra_tags) const
{
    auto& anim_manager = Kaputt::getSingleton()->anim_manager;
    auto  tag_result   = Tagger::tag(tagger_list, attacker, victim);
    tag_result.merge(extra_tags);

    std::erase_if(anims, [&](std::string_view edid) {
        StrSet exp_tags  = {};
        auto&  orig_tags = anim_manager.getTags(edid);

        for (const auto& [from, to] : tagexp_list)
            if (orig_tags.contains(from))
                exp_tags.merge(const_cast<StrSet&>(to));
        exp_tags.merge(const_cast<StrSet&>(orig_tags));

        return !(std::ranges::all_of(tag_result.required_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }) &&
                 std::ranges::none_of(tag_result.banned_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }));
    });
}
} // namespace kaputt
