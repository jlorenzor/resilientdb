from pathlib import Path


def patch_text(path: Path, old_text: str, new_text: str, label: str) -> None:
    data = path.read_text()
    normalized = data.replace("\r\n", "\n")
    if old_text not in normalized:
        raise SystemExit(f"{label} target block not found")
    path.write_text(normalized.replace(old_text, new_text))


commitment = Path("platform/consensus/ordering/hotstuff/commitment.cpp")
patch_text(
    commitment,
    '''  QC high_qc = GetHighQC(view_number - 1);
  LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_prepare"
             << " self=" << id_
             << " view=" << view_number
             << " high_qc_view=" << high_qc.node().info().view()
             << " high_qc_sigs=" << high_qc.signatures_size()
             << " user_data_size=" << user_request->node().data().size()
             << " proxy=" << user_request->node().proxy_id();
''',
    '''  QC high_qc = GetHighQC(view_number - 1);
  QC prepare_qc = message_manager_->GetPrepareQC();
  LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_highqc_candidate"
             << " self=" << id_
             << " view=" << view_number
             << " high_qc_view=" << high_qc.node().info().view()
             << " high_qc_sigs=" << high_qc.signatures_size()
             << " prepare_qc_view=" << prepare_qc.node().info().view()
             << " prepare_qc_sigs=" << prepare_qc.signatures_size();
  if (prepare_qc.node().info().view() > high_qc.node().info().view() ||
      (high_qc.signatures_size() == 0 && prepare_qc.signatures_size() > 0)) {
    LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_highqc_fallback_prepareqc"
               << " self=" << id_
               << " view=" << view_number
               << " from_high_qc_view=" << high_qc.node().info().view()
               << " to_prepare_qc_view=" << prepare_qc.node().info().view();
    high_qc = prepare_qc;
  }
  LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_prepare"
             << " self=" << id_
             << " view=" << view_number
             << " high_qc_view=" << high_qc.node().info().view()
             << " high_qc_sigs=" << high_qc.signatures_size()
             << " user_data_size=" << user_request->node().data().size()
             << " proxy=" << user_request->node().proxy_id();
''',
    "commitment",
)

manager = Path("platform/consensus/ordering/hotstuff/message_manager.cpp")
patch_text(
    manager,
    '''bool MessageManager::IsSaveNode(const HotStuffRequest& request) {
  std::unique_lock<std::mutex> lk(mutex_);
  return request.node().pre().hash() == lock_qc_.node().info().hash() ||
         request.qc().node().info().view() > lock_qc_.node().info().view();
}
''',
    '''bool MessageManager::IsSaveNode(const HotStuffRequest& request) {
  std::unique_lock<std::mutex> lk(mutex_);
  const bool pre_hash_matches =
      request.node().pre().hash() == lock_qc_.node().info().hash();
  const bool qc_view_is_newer =
      request.qc().node().info().view() > lock_qc_.node().info().view();
  LOG(ERROR) << "CHATAY_HS1_TRACE safe_node_check"
             << " request_node_view=" << request.node().info().view()
             << " qc_view=" << request.qc().node().info().view()
             << " lock_qc_view=" << lock_qc_.node().info().view()
             << " pre_hash_size=" << request.node().pre().hash().size()
             << " lock_hash_size=" << lock_qc_.node().info().hash().size()
             << " pre_hash_matches=" << pre_hash_matches
             << " qc_view_is_newer=" << qc_view_is_newer;
  return pre_hash_matches || qc_view_is_newer;
}
''',
    "message manager",
)
