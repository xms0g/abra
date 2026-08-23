#pragma once

template<typename T, typename KeyType>
T* ResourceManager::get(const KeyType& key) const {
	if constexpr (std::is_same_v<T, MeshMap>) {
		return const_cast<T*>(&mMeshesByEntity.at(key));
	} else if constexpr (std::is_same_v<T, MaterialMap>) {
		return const_cast<T*>(&mMaterialsByEntity.at(key));
	} else if constexpr (std::is_same_v<T, float>) {
		return const_cast<T*>(mTransformsByEntity.at(key).data());
	} else {
		static_assert(false, "Unsupported type for get().");
	}

	return nullptr;
}

template<typename T>
void ResourceManager::upload(size_t entityID, T& map) {
	if constexpr (std::is_same_v<T, MeshMap>) {
		mMeshesByEntity.emplace(entityID, std::move(map));
	} else if constexpr (std::is_same_v<T, MaterialMap>) {
		mMaterialsByEntity.emplace(entityID, std::move(map));
	} else if constexpr (std::is_same_v<T, std::vector<float> >) {
		mTransformsByEntity.emplace(entityID, std::move(map));
	}
}

