#pragma once

#include "pnl_calculator_concepts.h"
#include "pnl_calculator_types.h"
#include "pnl_calculator_accountingtraits.h"
#include "pnl_calculator_macros.h"
#include <unordered_map>
#include <deque>
#include <vector>
#include <ranges>
#include <algorithm>

namespace pnl::engine
{
    template <typename T>
    using PositionContainer = std::deque<T>;

    template <concepts::AccountingMethod AccountingTraits>
    class CACHE_LINE_ALIGNED PositionTracker
    {
    private:
        using traits_type = AccountingTraits;
        using position_container = PositionContainer<types::Position>;

        std::unordered_map<std::string, position_container> buy_positions_;
        std::unordered_map<std::string, position_container> sell_positions_;

        FORCE_INLINE double calculate_pnl(
            const types::Position& position,
            const types::Trade& trade,
            typename AccountingTraits::quantity_t quantity) const noexcept;

        double clear_positions_fifo(
            position_container& positions,
            const types::Trade& trade,
            typename AccountingTraits::quantity_t& remaining_quantity);

        double clear_positions_lifo(
            position_container& positions,
            const types::Trade& trade,
            typename AccountingTraits::quantity_t& remaining_quantity);

    public:
        RULE_OF_FIVE_MOVABLE(PositionTracker)

        PositionTracker();

        void add_position(
            const std::string& symbol,
            const types::Position& position,
            enums::TradeSide side);

        template <typename PnLCallback>
        requires std::invocable<PnLCallback, types::PnLResult>
        void process_trade(const types::Trade& trade, PnLCallback&& callback);
    };

    template <concepts::AccountingMethod AccountingTraits>
    class PnLCalculationEngine
    {
    private:
        using traits_type = AccountingTraits;
        using position_tracker_type = PositionTracker<AccountingTraits>;

        position_tracker_type position_tracker_;
        std::vector<types::PnLResult> results_;

    public:
        RULE_OF_FIVE_MOVABLE(PnLCalculationEngine)

        PnLCalculationEngine();

        template <concepts::TradeContainer Container>
        void process_trades(const Container& trades);

        template <std::ranges::input_range R>
        requires concepts::Trade<std::ranges::range_value_t<R>>
        void process_trades_range(R&& trades);

        [[nodiscard]] const std::vector<types::PnLResult>& get_results() const noexcept;
        [[nodiscard]] std::vector<types::PnLResult> extract_results() noexcept;

        void clear() noexcept;
        [[nodiscard]] std::size_t size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;
    };

    template <enums::AccountingType Method>
    auto create_engine();
}

#include "pnl_calculator_engine.hxx"