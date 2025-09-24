#pragma once

#include <cmath>

namespace pnl::engine
{
    template <concepts::AccountingMethod AccountingTraits>
    inline PositionTracker<AccountingTraits>::PositionTracker()
    {
        buy_positions_.reserve(AccountingTraits::default_reserve_size);
        sell_positions_.reserve(AccountingTraits::default_reserve_size);
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline void PositionTracker<AccountingTraits>::add_position(
        const std::string& symbol,
        const types::Position& position,
        enums::TradeSide side)
    {
        auto& container = (side == enums::TradeSide::BUY) ? buy_positions_[symbol] : sell_positions_[symbol];
        container.emplace_back(position);
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline double PositionTracker<AccountingTraits>::calculate_pnl(
        const types::Position& position,
        const types::Trade& trade,
        typename AccountingTraits::quantity_t quantity) const noexcept
    {
        return trade.is_buy()
               ? static_cast<double>(quantity) * (static_cast<double>(position.price()) - static_cast<double>(trade.price()))
               : static_cast<double>(quantity) * (static_cast<double>(trade.price()) - static_cast<double>(position.price()));
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline double PositionTracker<AccountingTraits>::clear_positions_fifo(
        position_container& positions,
        const types::Trade& trade,
        typename AccountingTraits::quantity_t& remaining_quantity)
    {
        double total_pnl = 0.0;

        while (!positions.empty() && remaining_quantity > 0) LIKELY
        {
            auto& position = positions.front();
            const typename AccountingTraits::quantity_t clear_quantity = std::min(remaining_quantity, position.quantity());

            total_pnl += calculate_pnl(position, trade, clear_quantity);
            remaining_quantity -= clear_quantity;
            position.reduce_quantity(clear_quantity);

            if (position.is_empty()) LIKELY
            {
                positions.pop_front();
            }
        }

        return total_pnl;
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline double PositionTracker<AccountingTraits>::clear_positions_lifo(
        position_container& positions,
        const types::Trade& trade,
        typename AccountingTraits::quantity_t& remaining_quantity)
    {
        double total_pnl = 0.0;

        while (!positions.empty() && remaining_quantity > 0) LIKELY
        {
            auto& position = positions.back();
            const typename AccountingTraits::quantity_t clear_quantity = std::min(remaining_quantity, position.quantity());

            total_pnl += calculate_pnl(position, trade, clear_quantity);
            remaining_quantity -= clear_quantity;
            position.reduce_quantity(clear_quantity);

            if (position.is_empty()) LIKELY
            {
                positions.pop_back();
            }
        }

        return total_pnl;
    }

    template <concepts::AccountingMethod AccountingTraits>
    template <typename PnLCallback>
    requires std::invocable<PnLCallback, types::PnLResult>
    inline void PositionTracker<AccountingTraits>::process_trade(
        const types::Trade& trade,
        PnLCallback&& callback)
    {
        const auto& symbol = trade.symbol();
        const auto opposite_side = trade.is_buy() ? enums::TradeSide::SELL : enums::TradeSide::BUY;

        auto& opposite_positions = (opposite_side == enums::TradeSide::BUY)
                                 ? buy_positions_[symbol]
                                 : sell_positions_[symbol];

        if (opposite_positions.empty()) LIKELY
        {
            add_position(symbol, types::Position{trade.price(), trade.quantity(), trade.timestamp()}, trade.side());
            return;
        }

        typename AccountingTraits::quantity_t remaining_quantity = trade.quantity();
        double total_pnl = 0.0;

        if constexpr (AccountingTraits::is_fifo)
        {
            total_pnl = clear_positions_fifo(opposite_positions, trade, remaining_quantity);
        }
        else
        {
            total_pnl = clear_positions_lifo(opposite_positions, trade, remaining_quantity);
        }

        if (remaining_quantity > 0) LIKELY
        {
            add_position(symbol, types::Position{trade.price(), remaining_quantity, trade.timestamp()}, trade.side());
        }

        if (std::abs(total_pnl) > constants::EPSILON) LIKELY
        {
            callback(types::PnLResult{trade.timestamp(), symbol, AccountingTraits::format_precision(total_pnl)});
        }
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline PnLCalculationEngine<AccountingTraits>::PnLCalculationEngine()
    {
        results_.reserve(AccountingTraits::default_reserve_size);
    }

    template <concepts::AccountingMethod AccountingTraits>
    template <concepts::TradeContainer Container>
    inline void PnLCalculationEngine<AccountingTraits>::process_trades(const Container& trades)
    {
        for (const auto& trade : trades)
        {
            position_tracker_.process_trade(trade, [this](const types::PnLResult& result)
            {
                results_.emplace_back(result);
            });
        }
    }

    template <concepts::AccountingMethod AccountingTraits>
    template <std::ranges::input_range R>
    requires concepts::Trade<std::ranges::range_value_t<R>>
    inline void PnLCalculationEngine<AccountingTraits>::process_trades_range(R&& trades)
    {
        for (auto&& trade : trades)
        {
            position_tracker_.process_trade(trade, [this](const types::PnLResult& result)
            {
                results_.emplace_back(result);
            });
        }
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline const std::vector<types::PnLResult>& PnLCalculationEngine<AccountingTraits>::get_results() const noexcept
    {
        return results_;
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline std::vector<types::PnLResult> PnLCalculationEngine<AccountingTraits>::extract_results() noexcept
    {
        return std::move(results_);
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline void PnLCalculationEngine<AccountingTraits>::clear() noexcept
    {
        results_.clear();
        position_tracker_ = position_tracker_type{};
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline std::size_t PnLCalculationEngine<AccountingTraits>::size() const noexcept
    {
        return results_.size();
    }

    template <concepts::AccountingMethod AccountingTraits>
    inline bool PnLCalculationEngine<AccountingTraits>::empty() const noexcept
    {
        return results_.empty();
    }

    template <enums::AccountingType Method>
    inline auto create_engine()
    {
        return PnLCalculationEngine<traits::AccountingTraits<Method>>{};
    }
}