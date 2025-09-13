//
// Created by fortwoone on 13/09/2025.
//

#include "backref_mgr.hpp"

namespace cpp_grep{
    // region BackRefText
    bool BackRefText::is_reserved() const{
        return reserved;
    }

    bool BackRefText::empty() const{
        return txt.empty();
    }

    string BackRefText::get_text() const{
        return txt;
    }

    void BackRefText::reserve(){
        reserved = true;
    }

    void BackRefText::free(){
        reserved = false;
    }

    void BackRefText::reset(){
        txt.clear();
        reserved = false;
    }

    void BackRefText::change_text(const string& new_text){
        txt = new_text;
    }
    // endregion

    // region BackRefManager
    BackRefManager::BackRefManager(ubyte size){
        back_ref_texts.resize(size);
    }

    string BackRefManager::get_text_at(ubyte index) const{
        return back_ref_texts.at(index).get_text();
    }

    bool BackRefManager::is_text_reserved_at(ubyte index) const{
        return back_ref_texts.at(index).is_reserved();
    }

    ubyte BackRefManager::size() const{
        return back_ref_texts.size();
    }

    ubyte BackRefManager::reserve_first_free_slot(){
        auto begin_iter = back_ref_texts.begin();
        auto end_iter = back_ref_texts.end();

        auto found = find_if_not(
            begin_iter,
            end_iter,
            [](const BackRefText& txt_obj){
                return txt_obj.is_reserved();
            }
        );

        if (found == end_iter){
            throw out_of_range("All slots are reserved.");
        }

        auto index = found - begin_iter;
        back_ref_texts.at(index).reserve();
        return static_cast<ubyte>(index);
    }

    void BackRefManager::set_text_at(ubyte index, const string& new_text){
        back_ref_texts.at(index).change_text(new_text);
    }

    void BackRefManager::reserve_at(ubyte index){
        back_ref_texts.at(index).reserve();
    }

    void BackRefManager::free_at(ubyte index){
        back_ref_texts.at(index).free();
    }

    void BackRefManager::resize(ubyte new_size){
        back_ref_texts.resize(new_size);
    }

    void BackRefManager::reset(){
        for (auto& txt_obj: back_ref_texts){
            txt_obj.reset();
        }
    }
    // endregion
}
