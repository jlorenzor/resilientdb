from pathlib import Path


def replace(path: Path, old: str, new: str, label: str) -> None:
    text = path.read_text().replace("\r\n", "\n")
    if old not in text:
        raise SystemExit(f"{label} target block not found")
    path.write_text(text.replace(old, new))


header = Path("platform/consensus/ordering/hotstuff/message_manager.h")
replace(
    header,
    """ private:
  QC lock_qc_, prepare_qc_;
  std::mutex mutex_;
};
""",
    """ private:
  QC lock_qc_, prepare_qc_;
  std::mutex mutex_;
  int64_t next_execute_seq_ = 1;
};
""",
    "message manager header",
)

source = Path("platform/consensus/ordering/hotstuff/message_manager.cpp")
replace(
    source,
    """int MessageManager::Commit(std::unique_ptr<HotStuffRequest> hotstuff_request) {
  std::unique_ptr<Request> execute_request = std::make_unique<Request>();
  execute_request->set_data(hotstuff_request->qc().node().data());
  execute_request->set_seq(hotstuff_request->qc().node().info().view());
  execute_request->set_proxy_id(hotstuff_request->qc().node().proxy_id());
  LOG(ERROR) << "CHATAY_HS1_TRACE message_manager_commit"
             << " seq=" << execute_request->seq()
             << " proxy=" << execute_request->proxy_id()
             << " data_size=" << execute_request->data().size();
  transaction_executor_->Commit(std::move(execute_request));
  return 0;
}
""",
    """int MessageManager::Commit(std::unique_ptr<HotStuffRequest> hotstuff_request) {
  std::unique_ptr<Request> execute_request = std::make_unique<Request>();
  const int64_t consensus_view = hotstuff_request->qc().node().info().view();
  int64_t execute_seq = 0;
  {
    std::unique_lock<std::mutex> lk(mutex_);
    execute_seq = next_execute_seq_++;
  }
  execute_request->set_data(hotstuff_request->qc().node().data());
  execute_request->set_seq(execute_seq);
  execute_request->set_proxy_id(hotstuff_request->qc().node().proxy_id());
  LOG(ERROR) << "CHATAY_HS1_TRACE message_manager_commit"
             << " execute_seq=" << execute_request->seq()
             << " consensus_view=" << consensus_view
             << " proxy=" << execute_request->proxy_id()
             << " data_size=" << execute_request->data().size();
  transaction_executor_->Commit(std::move(execute_request));
  return 0;
}
""",
    "message manager commit",
)
