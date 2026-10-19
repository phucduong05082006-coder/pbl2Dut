#pragma once

#include "entities/Ticket.h"
#include "repositories/Repository.h"

class TicketRepository final {
public:
    using value_type = Ticket;

    [[nodiscard]] bool empty() const noexcept { return records_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
    [[nodiscard]] bool isDirty() const noexcept { return records_.isDirty(); }
    void markSaved() noexcept { records_.markSaved(); }
    EntityId nextId() const { return records_.nextId(); }
    const Ticket* findById(EntityId id) const { return records_.findById(id); }
    bool insert(const Ticket& ticket) { return records_.insert(ticket); }

    template <typename Function>
    void forEach(Function function) const {
        records_.forEach(function);
    }

private:
    friend class ApplicationData;
    Repository<Ticket> records_;
};
