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

        FORCE_INLINE double calculate_pnl(const types::Position& position, const types::Trade& trade, typename AccountingTraits::quantity_t quantity) const noexcept
        {
            return trade.is_buy()
                   ? static_cast<double>(quantity) * (static_cast<double>(position.price()) - static_cast<double>(trade.price()))  // Selling against buy position
                   : static_cast<double>(quantity) * (static_cast<double>(trade.price()) - static_cast<double>(position.price())); // Buying against sell position
        }

        double clear_positions_fifo(position_container& positions, const types::Trade& trade, typename AccountingTraits::quantity_t& remaining_quantity)
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

        double clear_positions_lifo(position_container& positions, const types::Trade& trade, typename AccountingTraits::quantity_t& remaining_quantity)
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

    public:
        RULE_OF_FIVE_MOVABLE(PositionTracker)

        PositionTracker()
    	{
            buy_positions_.reserve(AccountingTraits::default_reserve_size);
            sell_positions_.reserve(AccountingTraits::default_reserve_size);
        }

        void add_position(const std::string& symbol, const types::Position& position, enums::TradeSide side)
    	{
            auto& container = (side == enums::TradeSide::BUY) ? buy_positions_[symbol] : sell_positions_[symbol];
            container.emplace_back(position);
        }

        template <typename PnLCallback>
        requires std::invocable<PnLCallback, types::PnLResult>
        void process_trade(const types::Trade& trade, PnLCallback&& callback)
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

        PnLCalculationEngine()
    	{
            results_.reserve(AccountingTraits::default_reserve_size);
        }

        template <concepts::TradeContainer Container>
        void process_trades(const Container& trades)
    	{
            for (const auto& trade : trades)
    		{
                position_tracker_.process_trade(trade, [this](const types::PnLResult& result)
    			{
                    results_.emplace_back(result);
                });
            }
        }

        template <std::ranges::input_range R>
        requires concepts::Trade<std::ranges::range_value_t<R>>
        void process_trades_range(R&& trades)
    	{
            for (auto&& trade : trades)
    		{
                position_tracker_.process_trade(trade, [this](const types::PnLResult& result)
    			{
                    results_.emplace_back(result);
                });
            }
        }

        [[nodiscard]] const std::vector<types::PnLResult>& get_results() const noexcept
    	{
            return results_;
        }

        [[nodiscard]] std::vector<types::PnLResult> extract_results() noexcept
    	{
            return std::move(results_);
        }

        void clear() noexcept
    	{
            results_.clear();
            position_tracker_ = position_tracker_type{};
        }

        [[nodiscard]] std::size_t size() const noexcept
    	{
            return results_.size();
        }

        [[nodiscard]] bool empty() const noexcept
    	{
            return results_.empty();
        }
    };

    template <enums::AccountingType Method>
    auto create_engine()
    {
        return PnLCalculationEngine<traits::AccountingTraits<Method>>{};
    }
}
